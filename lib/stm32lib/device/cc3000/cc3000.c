
#include "cc3000.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "host-driver/wlan.h"
#include "host-driver/hci.h"
#include "host-driver/cc3000_common.h"
#include "host-driver/socket.h"
#include "host-driver/nvmem.h"
#include "../../spi.h"
#include "../../rcc.h"
#include "../../hal/gpio.h"
#include "../../time.h"

#define MAXSSID           (32)
#define MAXLENGTHKEY      (32)  /* Cleared for 32 bytes by TI engineering 29/08/13 */

// The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
// for the purpose of detection of the overrun. The location of the memory where the magic number
// resides shall never be written. In case it is written - the overrun occured and either recevie function
// or send function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER (0xDE)

#define CC3000_SPI_STATE_POWERUP              (0)
#define CC3000_SPI_STATE_INITIALIZED          (1)
#define CC3000_SPI_STATE_IDLE                 (2)
#define CC3000_SPI_STATE_WRITE_IRQ            (3)
#define CC3000_SPI_STATE_WRITE_FIRST_PORTION  (4)
#define CC3000_SPI_STATE_WRITE_EOT            (5)
#define CC3000_SPI_STATE_READ_IRQ             (6)
#define CC3000_SPI_STATE_READ_FIRST_PORTION   (7)
#define CC3000_SPI_STATE_READ_EOT             (8)

#define CC3000_HEADERS_SIZE_EVNT              (SPI_HEADER_SIZE + 5)
#define CC3000_SPI_HEADER_SIZE                (5)

#define CC3000_READ                            (3)
#define CC3000_WRITE                           (1)
#define CC3000_HI(value)                       (((value) & 0xFF00) >> 8)
#define CC3000_LO(value)                       ((value) & 0x00FF)

#define CC3000_SUCCESS 0

// used by wlan.c
uint8_t wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];

void _cc3000_cs_assert(CC3000 *cc3000);
void _cc3000_cs_deassert(CC3000 *cc3000);
void _cc3000_en_assert(CC3000 *cc3000);
void _cc3000_en_deassert(CC3000 *cc3000);
void _cc3000_asyncCallback(long lEventType, char* data, unsigned char length);
char* _cc3000_sendFWPatches(unsigned long* length);
char* _cc3000_sendDriverPatches(unsigned long* length);
char* _cc3000_sendBootLoaderPatches(unsigned long* length);
long _cc3000_readWlanInterruptPin();
void _cc3000_wlanInterruptEnable();
void _cc3000_wlanInterruptDisable();
void _cc3000_writeWlanPin(unsigned char val);
uint8_t _cc3000_spiTransfer(CC3000 *cc3000, uint8_t d);
void _cc3000_spiReadHeader(CC3000 *cc3000);
void _cc3000_spiReadDataSynchronous(CC3000 *cc3000, uint8_t* data, uint16_t size);
void _cc3000_spiWriteDataSynchronous(CC3000 *cc3000, uint8_t* data, uint16_t size);
void _cc3000_ssiContReadOperation(CC3000 *cc3000);
bool _cc3000_spiReadDataCont(CC3000 *cc3000);
void _cc3000_spiTriggerRxProcessing(CC3000 *cc3000);
void _cc3000_spiFirstWrite(CC3000 *cc3000, uint8_t* buffer, uint16_t length);
bool _cc3000_scanSSIDs(uint32_t time);
bool _cc3000_connectOpen(const char* ssid);
bool _cc3000_connectSecure(const char* ssid, const char* key, int32_t secMode);
bool _cc3000_getFirmwareVersion(uint8_t* major, uint8_t* minor);

void cc3000_setupGpio(CC3000 *cc3000) {
  GPIO_InitParams gpio;

  printf("BEGIN CC3000 Setup GPIO\n");

  RCC_peripheralClockEnableForPort(cc3000->csPort);
  RCC_peripheralClockEnableForPort(cc3000->irqPort);
  RCC_peripheralClockEnableForPort(cc3000->enPort);

  GPIO_initParamsInit(&gpio);
  gpio.port = cc3000->csPort;
  gpio.pin = cc3000->csPin;
  gpio.mode = GPIO_Mode_output;
  gpio.pullUpDown = GPIO_PullUpDown_pullUp;
  gpio.speed = GPIO_Speed_high;
  gpio.outputType = GPIO_OutputType_pushPull;
  GPIO_init(&gpio);

  gpio.port = cc3000->enPort;
  gpio.pin = cc3000->enPin;
  gpio.pullUpDown = GPIO_PullUpDown_pullDown;
  GPIO_init(&gpio);

  GPIO_initParamsInit(&gpio);
  gpio.port = cc3000->irqPort;
  gpio.pin = cc3000->irqPin;
  gpio.mode = GPIO_Mode_input;
  gpio.speed = GPIO_Speed_high;
  GPIO_init(&gpio);

  _cc3000_cs_deassert(cc3000);
  _cc3000_en_deassert(cc3000);

  printf("END CC3000 Setup GPIO\n");
}

bool cc3000_setup(CC3000 *cc3000) {
  printf("BEGIN CC3000 Setup (patchReq: %d, connectPolicy: %d, deviceName: %s)\n", cc3000->patchReq, cc3000->connectPolicy, cc3000->deviceName);

  cc3000->pingReportNum = 0;
  cc3000->irqEnabled = false;
  memset(&cc3000->status, 0, sizeof(cc3000->status));
  memset(&cc3000->pingReport, 0, sizeof(cc3000->pingReport));
  for (int i = 0; i < MAX_SOCKETS; i++) {
    cc3000->sockets[i].closed = false;
  }
  cc3000->inIrq = false;

  wlan_init(
    _cc3000_asyncCallback,
    _cc3000_sendFWPatches,
    _cc3000_sendDriverPatches,
    _cc3000_sendBootLoaderPatches,
    _cc3000_readWlanInterruptPin,
    _cc3000_wlanInterruptEnable,
    _cc3000_wlanInterruptDisable,
    _cc3000_writeWlanPin
  );

  wlan_start(cc3000->patchReq);

  uint8_t firmwareMajor, firmwareMinor;
  if (_cc3000_getFirmwareVersion(&firmwareMajor, &firmwareMinor)) {
    printf("CC3000 firmware: %d.%d\n", firmwareMajor, firmwareMinor);
  } else {
    printf("failed to get firmware\n");
    return false;
  }

  // Check if we should erase previous stored connection details
  // (most likely written with data from the SmartConfig app)
  int connectToOpenAPs = (cc3000->connectPolicy & CC3000_CONNECT_POLICY_OPEN_AP) ? 1 : 0;
  int useFastConnect = (cc3000->connectPolicy & CC3000_CONNECT_POLICY_FAST) ? 1 : 0;
  int useProfiles = (cc3000->connectPolicy & CC3000_CONNECT_POLICY_PROFILES) ? 1 : 0;
  wlan_ioctl_set_connection_policy(connectToOpenAPs, useFastConnect, useProfiles);
  if (connectToOpenAPs == 0 && useFastConnect == 0 && useProfiles == 0) {
    // Delete previous profiles from memory
    wlan_ioctl_del_profile(255);
  }

  if (wlan_set_event_mask(HCI_EVNT_WLAN_UNSOL_INIT        |
                          //HCI_EVNT_WLAN_ASYNC_PING_REPORT |// we want ping reports
                          //HCI_EVNT_BSD_TCP_CLOSE_WAIT |
                          //HCI_EVNT_WLAN_TX_COMPLETE |
                          HCI_EVNT_WLAN_KEEPALIVE) != CC3000_SUCCESS) {
    printf("WLAN Set Event Mask FAIL\n");
    return false;
  }

  // Wait for re-connection if we're using SmartConfig data
  if (cc3000->connectPolicy & CC3000_CONNECT_POLICY_SMART_CONFIG) {
    // Wait for a connection
    uint32_t timeout = 0;
    while (!cc3000->status.isConnected) {
      _cc3000_irqPoll();
      if (timeout > WLAN_CONNECT_TIMEOUT) {
        return false;
      }
      timeout += 10;
      sleep_ms(10);
    }

    sleep_ms(1000);
    if (cc3000->status.hasDHCP) {
      mdnsAdvertiser(1, (char*) cc3000->deviceName, strlen(cc3000->deviceName));
    }
  }

  printf("END CC3000 Setup\n");
  return true;
}

SOCKET cc3000_connectUDP(uint32_t destIP, uint16_t destPort) {
  sockaddr socketAddress;
  SOCKET sock;

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (-1 == sock) {
    return -1;
  }

  memset(&socketAddress, 0x00, sizeof(socketAddress));
  socketAddress.sa_family = AF_INET;
  socketAddress.sa_data[0] = (destPort & 0xFF00) >> 8;  // Set the Port Number
  socketAddress.sa_data[1] = (destPort & 0x00FF);
  socketAddress.sa_data[2] = destIP >> 24;
  socketAddress.sa_data[3] = destIP >> 16;
  socketAddress.sa_data[4] = destIP >> 8;
  socketAddress.sa_data[5] = destIP;

  if (-1 == connect(sock, &socketAddress, sizeof(socketAddress))) {
    closesocket(sock);
    return -1;
  }

  return sock;
}

bool cc3000_socket_connected(CC3000* cc3000, SOCKET sock) {
  if (sock < 0) {
    return false;
  }

  if (!cc3000_socket_available(cc3000, sock) && cc3000->sockets[sock].closed == true) {
    //if (CC3KPrinter != 0) CC3KPrinter->println("No more data, and closed!");
    closesocket(sock);
    cc3000->sockets[sock].closed = false;
    sock = -1;
    return false;
  }

  return true;
}

int cc3000_socket_available(CC3000* cc3000, SOCKET sockIndex) {
  // not open!
  if (sockIndex < 0) {
    return 0;
  }

  _CC3000Socket* sock = &cc3000->sockets[sockIndex];

  if ((sock->bufferSize > 0) // we have some data in the internal buffer
      && (sock->rxBufferIndex < sock->bufferSize)) {  // we havent already spit it all out
    return (sock->bufferSize - sock->rxBufferIndex);
  }

  // do a select() call on this socket
  timeval timeout;
  fd_set fd_read;

  memset(&fd_read, 0, sizeof(fd_read));
  FD_SET(sockIndex, &fd_read);

  timeout.tv_sec = 0;
  timeout.tv_usec = 5000; // 5 millisec

  int16_t s = select(sockIndex + 1, &fd_read, NULL, NULL, &timeout);
  if (s == 1) {
    return 1; // some data is available to read
  }
  return 0; // no data is available
}

int cc3000_socket_write(SOCKET sock, const void* buf, uint16_t len, uint32_t flags) {
  return send(sock, buf, len, flags);
}

int cc3000_socket_read(SOCKET sock, void* buf, uint16_t len, uint32_t flags) {
  return recv(sock, buf, len, flags);
}

bool cc3000_socket_close(SOCKET sock) {
  return closesocket(sock);
}


void _cc3000_cs_assert(CC3000 *cc3000) {
  GPIO_resetBits(cc3000->csPort, cc3000->csPin);
}

void _cc3000_cs_deassert(CC3000 *cc3000) {
  GPIO_setBits(cc3000->csPort, cc3000->csPin);
}

void _cc3000_en_assert(CC3000 *cc3000) {
  GPIO_setBits(cc3000->enPort, cc3000->enPin);
}

void _cc3000_en_deassert(CC3000 *cc3000) {
  GPIO_resetBits(cc3000->enPort, cc3000->enPin);
}

extern void SpiReceiveHandler(void* pvBuffer);

bool _cc3000_getFirmwareVersion(uint8_t* major, uint8_t* minor) {
  uint8_t fwpReturn[2];

  if (nvmem_read_sp_version(fwpReturn) != CC3000_SUCCESS) {
    printf("Unable to read the firmware version\n");
    return false;
  }

  *major = fwpReturn[0];
  *minor = fwpReturn[1];

  return true;
}

bool cc3000_deleteAllProfiles() {
  printf("deleting profiles\n");
  return wlan_ioctl_del_profile(255) == 0;
}

bool cc3000_addProfile(const char* ssid, const char* key, uint8_t secmode) {
  uint32_t ulPriority = 0;

  // taken from wlan.c WPA smart config example
  uint32_t ulPairwiseCipher_Or_TxKeyLen = 0x18;
  uint32_t ulGroupCipher_TxKeyIndex = 0x1e;
  uint32_t ulKeyMgmt = 0x2;

  int32_t result = wlan_add_profile(secmode, (uint8_t*)ssid, strlen(ssid), NULL, ulPriority, ulPairwiseCipher_Or_TxKeyLen, ulGroupCipher_TxKeyIndex, ulKeyMgmt, (uint8_t*)key, strlen(key));
  if (result == -1) {
    printf("failed to add profile %ld\n", result);
    return false;
  }
  return true;
}

bool cc3000_finishAddProfileAndConnect(CC3000* cc3000) {
  int connectToOpenAPs = 1;
  int useFastConnect = 1;
  int useProfiles = 1;
  if (wlan_ioctl_set_connection_policy(connectToOpenAPs, useFastConnect, useProfiles) != CC3000_SUCCESS) {
    printf("failed to set connection policy\n");
    return false;
  }

  wlan_stop();
  sleep_ms(100);

  bool patchAvailable = false;
  wlan_start(patchAvailable);

  int16_t timer = WLAN_CONNECT_TIMEOUT;
  while (!cc3000->status.isConnected) {
    sleep_ms(10);
    timer -= 10;
    if (timer <= 0) {
      printf("Timed out!\n");
      return false;
    }
    _cc3000_irqPoll();
  }
  return true;
}

bool cc3000_connectToAP(CC3000* cc3000, const char* ssid, const char* key, uint8_t secmode) {
  int16_t timer;

  do {
    _cc3000_irqPoll();

    /* MEME: not sure why this is absolutely required but the cc3k freaks
       if you dont. maybe bootup delay? */
    // Setup a 4 second SSID scan
    _cc3000_scanSSIDs(4000);
    // Wait for results
    sleep_ms(4500);
    _cc3000_scanSSIDs(0);

    /* Attempt to connect to an access point */
    printf("Connecting to '%s' (key: '%s', security: '%d')\n", ssid, key, secmode);
    if ((secmode == 0) || (strlen(key) == 0)) {
      /* Connect to an unsecured network */
      if (!_cc3000_connectOpen(ssid)) {
        printf("Failed to connect\n");
        continue;
      }
    } else {
      /* Connect to a secure network using WPA2, etc */
      if (!_cc3000_connectSecure(ssid, key, secmode)) {
        printf("Failed to connect\n");
        continue;
      }
    }

    timer = WLAN_CONNECT_TIMEOUT;

    /* Wait around a bit for the async connected signal to arrive or timeout */
    printf("Waiting to connect...\n");
    while ((timer > 0) && !cc3000->status.isConnected) {
      _cc3000_irqPoll();
      sleep_ms(10);
      timer -= 10;
    }
    if (timer <= 0) {
      printf("Timed out!\n");
    }
  } while (!cc3000->status.isConnected);

  return true;
}

bool cc3000_checkDHCP(CC3000* cc3000) {
  // Ugly hack to fix UDP issues with the 1.13 firmware by calling
  // gethostbyname on localhost.  The output is completely ignored
  // but for some reason this call is necessary or else UDP won't
  // work.  See this thread from TI for more details and the genesis
  // of the workaround: http://e2e.ti.com/support/wireless_connectivity/f/851/t/342177.aspx
  // Putting this in checkDHCP is a nice way to make it just work
  // for people without any need to add to their sketch.
  if (cc3000->status.hasDHCP) {
    uint32_t output;
    gethostbyname("localhost", 9, &output);
  }
  return cc3000->status.hasDHCP;
}

bool cc3000_getIPAddress(CC3000* cc3000, uint32_t* retip, uint32_t* netmask, uint32_t* gateway, uint32_t* dhcpserv, uint32_t* dnsserv) {
  if (!cc3000->status.isConnected) {
    return false;
  }
  if (!cc3000->status.hasDHCP) {
    return false;
  }

  tNetappIpconfigRetArgs ipconfig;
  netapp_ipconfig(&ipconfig);

  /* If byte 1 is 0 we don't have a valid address */
  if (ipconfig.aucIP[3] == 0) {
    return false;
  }

  memcpy(retip, ipconfig.aucIP, 4);
  memcpy(netmask, ipconfig.aucSubnetMask, 4);
  memcpy(gateway, ipconfig.aucDefaultGateway, 4);
  memcpy(dhcpserv, ipconfig.aucDHCPServer, 4);
  memcpy(dnsserv, ipconfig.aucDNSServer, 4);

  return true;
}

bool cc3000_getMacAddress(uint8_t* macAddress) {
  return nvmem_read(NVMEM_MAC_FILEID, 6, 0, macAddress) == CC3000_SUCCESS;
}

char* cc3000_ipToString(uint32_t ip, char* buffer) {
  sprintf(buffer, "%d.%d.%d.%d", (uint8_t)(ip >> 24), (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip >> 0));
  return buffer;
}

bool _cc3000_scanSSIDs(uint32_t time) {
  const unsigned long intervalTime[16] = { 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000 };

  // We can abort a scan with a time of 0
  if (time) {
    printf("Started AP/SSID scan\n");
  }

  // Set  SSID Scan params to includes channels above 11
  if (wlan_ioctl_set_scan_params(time, 20, 100, 5, 0x1FFF, -120, 0, 300, (unsigned long*) &intervalTime) != CC3000_SUCCESS) {
    printf("Failed setting params for SSID scan\n");
    return false;
  }

  return true;
}

bool _cc3000_connectOpen(const char* ssid) {
  if (wlan_ioctl_set_connection_policy(0, 0, 0) != CC3000_SUCCESS) {
    printf("Failed to set connection policy\n");
    return false;
  }
  sleep_ms(500);
  if (wlan_connect(WLAN_SEC_UNSEC, (char*)ssid, strlen(ssid), 0 , NULL, 0) != CC3000_SUCCESS) {
    printf("SSID connection failed\n");
    return false;
  }
  return true;
}

bool _cc3000_connectSecure(const char* ssid, const char* key, int32_t secMode) {
  if ((secMode < 0) || (secMode > 3)) {
    printf("Security mode must be between 0 and 3\n");
    return false;
  }

  if (strlen(ssid) > MAXSSID) {
    printf("SSID length must be < %d", MAXSSID);
    return false;
  }

  if (strlen(key) > MAXLENGTHKEY) {
    printf("Key length must be < %d", MAXLENGTHKEY);
    return false;
  }

  if (wlan_ioctl_set_connection_policy(0, 0, 0) != CC3000_SUCCESS) {
    printf("Failed setting the connection policy\n");
    return false;
  };
  sleep_ms(500);
  if (wlan_connect(secMode, (char*)ssid, strlen(ssid), NULL, (unsigned char*)key, strlen(key)) != CC3000_SUCCESS) {
    printf("SSID connection failed\n");
    return false;
  }

  /* Wait for 'HCI_EVNT_WLAN_UNSOL_CONNECT' in CC3000_UsynchCallback */
  return true;
}

void _cc3000_asyncCallback(long lEventType, char* data, unsigned char length) {
  CC3000 *_cc3000 = CC3000_GET;

  if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE) {
    _cc3000->status.isSmartConfigFinished = true;
  } else if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT) {
    _cc3000->status.isConnected = true;
  } else if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT) {
    _cc3000->status.isConnected = false;
    _cc3000->status.hasDHCP = false;
  } else if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP) {
    _cc3000->status.hasDHCP = true;
  } else if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN) {
    _cc3000->status.okToShutdown = true;
  } else if (lEventType == HCI_EVNT_WLAN_ASYNC_PING_REPORT) {
    _cc3000->pingReportNum++;
    memcpy(&_cc3000->pingReport, data, length);
  } else if (lEventType == HCI_EVNT_BSD_TCP_CLOSE_WAIT) {
    uint8_t socketnum;
    socketnum = data[0];
    if (socketnum < MAX_SOCKETS) {
      _cc3000->sockets[socketnum].closed = true;
    }
  }
}

char* _cc3000_sendFWPatches(unsigned long* length) {
  *length = 0;
  return NULL;
}

char* _cc3000_sendDriverPatches(unsigned long* length) {
  *length = 0;
  return NULL;
}

char* _cc3000_sendBootLoaderPatches(unsigned long* length) {
  *length = 0;
  return NULL;
}

long _cc3000_readWlanInterruptPin() {
  CC3000 *_cc3000 = CC3000_GET;
  return GPIO_readInputBit(_cc3000->irqPort, _cc3000->irqPin) == GPIO_Bit_set;
}

void _cc3000_wlanInterruptEnable() {
  CC3000 *_cc3000 = CC3000_GET;
  EXTI_InitParams exti;

  GPIO_EXTILineConfig(_cc3000->irqPort, _cc3000->irqPin);

  _cc3000->gpioInterruptLine = EXTI_getLineForGpio(_cc3000->irqPort, _cc3000->irqPin);
  _cc3000->gpioIrq = EXTI_getIRQForGpio(_cc3000->irqPort, _cc3000->irqPin);

  EXTI_initParamsInit(&exti);
  exti.line = _cc3000->gpioInterruptLine;
  exti.trigger = EXTI_Trigger_falling;
  EXTI_enable(&exti);
  GPIO_EXTILineConfig(_cc3000->irqPort, _cc3000->irqPin); // must be called after EXTI_enable
  NVIC_EnableIRQ(_cc3000->gpioIrq);

  _cc3000->irqEnabled = true;
}

void _cc3000_wlanInterruptDisable() {
  CC3000 *_cc3000 = CC3000_GET;
  NVIC_DisableIRQ(_cc3000->gpioIrq);

  _cc3000->irqEnabled = false;
}

void _cc3000_writeWlanPin(unsigned char val) {
  CC3000 *_cc3000 = CC3000_GET;
  if (val) {
    GPIO_setBits(_cc3000->enPort, _cc3000->enPin);
  } else {
    GPIO_resetBits(_cc3000->enPort, _cc3000->enPin);
  }
}

uint8_t _cc3000_spiTransfer(CC3000 *cc3000, uint8_t d) {
  return SPI_transfer(cc3000->spi, d);
}

void _cc3000_spiReadHeader(CC3000 *cc3000) {
  _cc3000_spiReadDataSynchronous(cc3000, cc3000->spiInformation.rxPacket, CC3000_HEADERS_SIZE_EVNT);
}

void _cc3000_spiReadDataSynchronous(CC3000 *cc3000, uint8_t* data, uint16_t size) {
  unsigned short i = 0;

  for (i = 0; i < size; i ++) {
    data[i] = _cc3000_spiTransfer(cc3000, 0x03);
  }
}

void _cc3000_ssiContReadOperation(CC3000 *cc3000) {
  /* The header was read - continue with  the payload read */
  if (_cc3000_spiReadDataCont(cc3000)) {
    /* All the data was read - finalize handling by switching to teh task
     *  and calling from task Event Handler */
    _cc3000_spiTriggerRxProcessing(cc3000);
  }
}

void _cc3000_spiTriggerRxProcessing(CC3000 *cc3000) {
  /* Trigger Rx processing */
  _cc3000_wlanInterruptDisable();
  _cc3000_cs_deassert(cc3000);

  /* The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
   * for the purpose of detection of the overrun. If the magic number is overriten - buffer overrun
   * occurred - and we will stuck here forever! */
  if (cc3000->spiInformation.rxPacket[CC3000_RX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER) {
    /* You've got problems if you're here! */
    printf("CC3000: ERROR - magic number missing!\n");
    while (1);
  }

  cc3000->spiInformation.spiState = CC3000_SPI_STATE_IDLE;
  cc3000->spiInformation.spiRxHandler(cc3000->spiInformation.rxPacket + SPI_HEADER_SIZE);
}

bool _cc3000_spiReadDataCont(CC3000 *cc3000) {
  long data_to_recv;
  uint8_t* evnt_buff, type;

  /* Determine what type of packet we have */
  evnt_buff = cc3000->spiInformation.rxPacket;
  data_to_recv = 0;
  STREAM_TO_UINT8((uint8_t*)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET, type);

  switch (type) {
    case HCI_TYPE_DATA: {
      /* We need to read the rest of data.. */
      STREAM_TO_UINT16((char*)(evnt_buff + SPI_HEADER_SIZE), HCI_DATA_LENGTH_OFFSET, data_to_recv);
      if (!((CC3000_HEADERS_SIZE_EVNT + data_to_recv) & 1)) {
        data_to_recv++;
      }

      if (data_to_recv) {
        _cc3000_spiReadDataSynchronous(cc3000, evnt_buff + CC3000_HEADERS_SIZE_EVNT, data_to_recv);
      }
      break;
    }
    case HCI_TYPE_EVNT: {
      /* Calculate the rest length of the data */
      STREAM_TO_UINT8((char*)(evnt_buff + SPI_HEADER_SIZE), HCI_EVENT_LENGTH_OFFSET, data_to_recv);
      data_to_recv -= 1;

      /* Add padding byte if needed */
      if ((CC3000_HEADERS_SIZE_EVNT + data_to_recv) & 1) {
        data_to_recv++;
      }

      if (data_to_recv) {
        _cc3000_spiReadDataSynchronous(cc3000, evnt_buff + CC3000_HEADERS_SIZE_EVNT, data_to_recv);
      }

      cc3000->spiInformation.spiState = CC3000_SPI_STATE_READ_EOT;
      break;
    }
  }

  return true;
}

void _cc3000_spiWriteDataSynchronous(CC3000 *cc3000, uint8_t* data, uint16_t size) {
  uint16_t loc;
  for (loc = 0; loc < size; loc ++) {
    _cc3000_spiTransfer(cc3000, data[loc]);
  }
}

void _cc3000_spiOpen(_cc3000_spiHandleRx rxHandler) {
  CC3000 *_cc3000 = CC3000_GET;
  
  _cc3000->spiInformation.spiState = CC3000_SPI_STATE_POWERUP;

  memset(_cc3000->spi_buffer, 0, sizeof(_cc3000->spi_buffer));
  memset(wlan_tx_buffer, 0, sizeof(wlan_tx_buffer));

  _cc3000->spiInformation.spiRxHandler = rxHandler;
  _cc3000->spiInformation.txPacketLength = 0;
  _cc3000->spiInformation.txPacket = NULL;
  _cc3000->spiInformation.rxPacket = _cc3000->spi_buffer;
  _cc3000->spiInformation.rxPacketLength = 0;

  _cc3000->spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
  wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

  /* Enable interrupt on the GPIO pin of WLAN IRQ */
  tSLInformation.WlanInterruptEnable();
}

void _cc3000_spiClose() {
  CC3000 *_cc3000 = CC3000_GET;

  if (_cc3000->spiInformation.rxPacket) {
    _cc3000->spiInformation.rxPacket = 0;
  }

  _cc3000_wlanInterruptDisable();
}

void _cc3000_spiResume() {
  _cc3000_wlanInterruptEnable();
}

void _cc3000_spiFirstWrite(CC3000 *cc3000, uint8_t* buffer, uint16_t length) {
  /* Workaround for the first transaction */
  _cc3000_cs_assert(cc3000);

  /* delay (stay low) for ~50us */
  sleep_ms(1);

  /* SPI writes first 4 bytes of data */
  _cc3000_spiWriteDataSynchronous(cc3000, buffer, 4);

  sleep_ms(1);

  _cc3000_spiWriteDataSynchronous(cc3000, buffer + 4, length - 4);

  /* From this point on - operate in a regular manner */
  cc3000->spiInformation.spiState = CC3000_SPI_STATE_IDLE;

  _cc3000_cs_deassert(cc3000);
}

void _cc3000_spiWrite(uint8_t* buffer, uint16_t length) {
  CC3000 *_cc3000 = CC3000_GET;
  uint8_t ucPad = 0;

  /* Figure out the total length of the packet in order to figure out if there is padding or not */
  if (!(length & 0x0001)) {
    ucPad++;
  }

  buffer[0] = CC3000_WRITE;
  buffer[1] = CC3000_HI(length + ucPad);
  buffer[2] = CC3000_LO(length + ucPad);
  buffer[3] = 0;
  buffer[4] = 0;

  length += (SPI_HEADER_SIZE + ucPad);

  /* The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
   * for the purpose of overrun detection. If the magic number is overwritten - buffer overrun
   * occurred - and we will be stuck here forever! */
  if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER) {
    printf("CC3000: Error - No magic number found in SpiWrite\n");
    while (1);
  }

  if (_cc3000->spiInformation.spiState == CC3000_SPI_STATE_POWERUP) {
    while (_cc3000->spiInformation.spiState != CC3000_SPI_STATE_INITIALIZED);
  }

  if (_cc3000->spiInformation.spiState == CC3000_SPI_STATE_INITIALIZED) {
    /* This is time for first TX/RX transactions over SPI: the IRQ is down - so need to send read buffer size command */
    _cc3000_spiFirstWrite(_cc3000, buffer, length);
  } else {
    /* We need to prevent here race that can occur in case two back to back packets are sent to the
     * device, so the state will move to IDLE and once again to not IDLE due to IRQ */
    tSLInformation.WlanInterruptDisable();

    while (_cc3000->spiInformation.spiState != CC3000_SPI_STATE_IDLE);

    _cc3000->spiInformation.spiState = CC3000_SPI_STATE_WRITE_IRQ;
    _cc3000->spiInformation.txPacket = buffer;
    _cc3000->spiInformation.txPacketLength = length;

    /* Assert the CS line and wait till SSI IRQ line is active and then initialize write operation */
    _cc3000_cs_assert(_cc3000);

    /* Re-enable IRQ - if it was not disabled - this is not a problem... */
    tSLInformation.WlanInterruptEnable();

    /* Check for a missing interrupt between the CS assertion and enabling back the interrupts */
    if (tSLInformation.ReadWlanInterruptPin() == 0) {
      _cc3000_spiWriteDataSynchronous(_cc3000, _cc3000->spiInformation.txPacket, _cc3000->spiInformation.txPacketLength);

      _cc3000->spiInformation.spiState = CC3000_SPI_STATE_IDLE;

      _cc3000_cs_deassert(_cc3000);
#ifdef SPI_HAS_TRANSACTION
      WlanInterruptEnable();
#endif
    }
  }

  /* Due to the fact that we are currently implementing a blocking situation
   * here we will wait till end of transaction */
  while (CC3000_SPI_STATE_IDLE != _cc3000->spiInformation.spiState);
}

void _cc3000_irqPoll(CC3000 *cc3000) {
  if (_cc3000_readWlanInterruptPin() && !cc3000->inIrq && cc3000->irqEnabled) {
    _cc3000_irq();
  }
}

void _cc3000_irq(CC3000 *cc3000) {
  cc3000->inIrq = true;

  if (cc3000->spiInformation.spiState == CC3000_SPI_STATE_POWERUP) {
    /* IRQ line was low ... perform a callback on the HCI Layer */
    cc3000->spiInformation.spiState = CC3000_SPI_STATE_INITIALIZED;
  } else if (cc3000->spiInformation.spiState == CC3000_SPI_STATE_IDLE) {
    cc3000->spiInformation.spiState = CC3000_SPI_STATE_READ_IRQ;

    /* IRQ line goes down - start reception */
    _cc3000_cs_assert(cc3000);

    // Wait for TX/RX Compete which will come as DMA interrupt
    _cc3000_spiReadHeader(cc3000);
    cc3000->spiInformation.spiState = CC3000_SPI_STATE_READ_EOT;
    _cc3000_ssiContReadOperation(cc3000);
  } else if (cc3000->spiInformation.spiState == CC3000_SPI_STATE_WRITE_IRQ) {
    _cc3000_spiWriteDataSynchronous(cc3000, cc3000->spiInformation.txPacket, cc3000->spiInformation.txPacketLength);
    cc3000->spiInformation.spiState = CC3000_SPI_STATE_IDLE;
    _cc3000_cs_deassert(cc3000);
  }

  cc3000->inIrq = false;
}

uint32_t inet_addr(const char* cp) {
  in_addr val;
  if (inet_aton(cp, &val)) {
    return (val.s_addr);
  }
  return (INADDR_NONE);
}

int inet_aton(const char* cp, in_addr* addr) {
  uint32_t val;
  int base, n;
  int c;
  uint parts[4];
  uint* pp = parts;

  c = *cp;
  for (;;) {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, isdigit=decimal.
     */
    if (!isdigit(c)) {
      return (0);
    }
    val = 0;
    base = 10;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16, c = *++cp;
      } else {
        base = 8;
      }
    }
    for (;;) {
      if (isascii(c) && isdigit(c)) {
        val = (val * base) + (c - '0');
        c = *++cp;
      } else if (base == 16 && isascii(c) && isxdigit(c)) {
        val = (val << 4) | (c + 10 - (islower(c) ? 'a' : 'A'));
        c = *++cp;
      } else {
        break;
      }
    }
    if (c == '.') {
      /*
       * Internet format:
       *	a.b.c.d
       *	a.b.c	(with c treated as 16 bits)
       *	a.b	(with b treated as 24 bits)
       */
      if (pp >= parts + 3) {
        return (0);
      }
      *pp++ = val;
      c = *++cp;
    } else {
      break;
    }
  }
  /*
   * Check for trailing characters.
   */
  if (c != '\0' && (!isascii(c) || !isspace(c))) {
    return (0);
  }
  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  n = pp - parts + 1;
  switch (n) {
    case 0:
      return (0); /* initial nondigit */

    case 1: /* a -- 32 bits */
      break;

    case 2: /* a.b -- 8.24 bits */
      if (val > 0xffffff) {
        return (0);
      }
      val |= parts[0] << 24;
      break;

    case 3: /* a.b.c -- 8.8.16 bits */
      if (val > 0xffff) {
        return (0);
      }
      val |= (parts[0] << 24) | (parts[1] << 16);
      break;

    case 4: /* a.b.c.d -- 8.8.8.8 bits */
      if (val > 0xff) {
        return (0);
      }
      val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
      break;
  }
  if (addr) {
    addr->s_addr = htonl(val);
  }
  return (1);
}


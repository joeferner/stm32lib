
#ifndef _CC3000_H_
#define _CC3000_H_

#include <platform_config.h>

#include "host-driver/cc3000_common.h"
#include "host-driver/wlan.h"
#include "host-driver/socket.h"
#include "host-driver/netapp.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../../exti.h"

#ifndef MAX_SOCKETS
# define MAX_SOCKETS 4
#endif

#ifndef WLAN_CONNECT_TIMEOUT
# define WLAN_CONNECT_TIMEOUT 10000  /* how long to wait, in milliseconds */
#endif

#ifndef CC3000_RX_BUFFER_SIZE
# define CC3000_RX_BUFFER_SIZE 64
#endif

typedef enum {
  CC3000_ConnectPolicy_openAP =0x01,
  CC3000_ConnectPolicy_fast = 0x02,
  CC3000_ConnectPolicy_profiles = 0x04,
  CC3000_ConnectPolicy_smartConfig = 0x10
} CC3000_ConnectPolicy;

typedef int32_t SOCKET;

typedef void (*_cc3000_spiHandleRx)(void* p);
typedef void (*_cc3000_spiHandleTx)();

extern uint8_t wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];

typedef struct {
  bool isSmartConfigFinished: 1;
  bool isConnected: 1;
  bool hasDHCP: 1;
  bool okToShutdown: 1;
} _CC3000Status;

typedef struct {
  _cc3000_spiHandleRx spiRxHandler;
  uint16_t txPacketLength;
  uint16_t rxPacketLength;
  uint16_t spiState;
  uint8_t* txPacket;
  uint8_t* rxPacket;
} _CC3000SpiInformation;

typedef struct {
  bool closed;
  uint8_t rxBuffer[CC3000_RX_BUFFER_SIZE];
  uint8_t rxBufferIndex;
  int16_t bufferSize;
} _CC3000Socket;

typedef struct {
  bool patchReq;
  CC3000_ConnectPolicy connectPolicy;
  const char* deviceName;
  _CC3000Status status;
  _CC3000SpiInformation spiInformation;
  _CC3000Socket sockets[MAX_SOCKETS];
  uint8_t pingReportNum;
  netapp_pingreport_args_t pingReport;
  bool irqEnabled;
  bool inIrq;
  uint8_t spi_buffer[CC3000_RX_BUFFER_SIZE];

  SPI_TypeDef *spi;
  GPIO_Port csPort;
  GPIO_Pin csPin;
  GPIO_Port irqPort;
  GPIO_Pin irqPin;
  GPIO_Port enPort;
  GPIO_Pin enPin;
  EXTI_Line gpioInterruptLine;
  IRQn_Type gpioIrq;
} CC3000;

#ifndef CC3000_GET
extern CC3000 cc3000;
# define CC3000_GET (&cc3000)
#endif

void cc3000_setupGpio(CC3000 *cc3000);
bool cc3000_setup(CC3000 *cc3000);
bool cc3000_connectToAP(CC3000* cc3000, const char* ssid, const char* key, uint8_t secmode);
bool cc3000_addProfile(const char* ssid, const char* key, uint8_t secmode);
bool cc3000_deleteAllProfiles();
bool cc3000_finishAddProfileAndConnect(CC3000* cc3000);
bool cc3000_checkDHCP(CC3000* cc3000);
bool cc3000_getIPAddress(CC3000* cc3000, uint32_t* retip, uint32_t* netmask, uint32_t* gateway, uint32_t* dhcpserv, uint32_t* dnsserv);
bool cc3000_getMacAddress(uint8_t* macAddress);
char* cc3000_ipToString(uint32_t ip, char* buffer);
SOCKET cc3000_connectUDP(uint32_t destIP, uint16_t destPort);

bool cc3000_socket_connected(CC3000* cc3000, SOCKET sock);
int cc3000_socket_available(CC3000* cc3000, SOCKET sock);
int cc3000_socket_write(SOCKET sock, const void* buf, uint16_t len, uint32_t flags);
int cc3000_socket_read(SOCKET sock, void* buf, uint16_t len, uint32_t flags);
bool cc3000_socket_close(SOCKET sock);

void _cc3000_irq();

#ifndef INADDR_NONE
# define INADDR_NONE ((uint32_t) 0xffffffff)
#endif
#ifndef INADDR_ANY
# define INADDR_ANY ((uint32_t) 0x00000000)
#endif

uint32_t inet_addr(const char* cp);
int inet_aton(const char* cp, in_addr* addr);

// used by CC3000 host driver
void _cc3000_spiOpen(_cc3000_spiHandleRx rxHandler);
void _cc3000_spiClose();
void _cc3000_irqPoll();
void _cc3000_spiResume();
void _cc3000_spiWrite(uint8_t* buffer, uint16_t length);

#endif

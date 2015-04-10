
#include "enc28j60.h"
#include "../../rcc.h"
#include "../../time.h"
#include "../../utils.h"
#include <net/ip/uip.h>
#include <net/ipv4/uip_arp.h>
#include <net/ip/tcpip.h>
#include "enc28j60_constants.h"

uint16_t _enc28j60_receivePacket(ENC28J60 *enc28j60, uint8_t *buffer, uint16_t bufferLength);

void _enc28j60_packetSend(ENC28J60 *enc28j60, uint8_t *packet1, uint16_t packet1Length, uint8_t *packet2, uint16_t packet2Length);
void _enc28j60_writeOp(ENC28J60 *enc28j60, uint8_t op, uint8_t address, uint8_t data);
void _enc28j60_writeRegPair(ENC28J60 *enc28j60, uint8_t address, uint16_t data);
void _enc28j60_writeReg(ENC28J60 *enc28j60, uint8_t address, uint8_t data);
void _enc28j60_phyWrite(ENC28J60 *enc28j60, uint8_t address, uint16_t data);
uint16_t _enc28j60_phyRead(ENC28J60 *enc28j60, uint8_t address);
void _enc28j60_setBank(ENC28J60 *enc28j60, uint8_t address);
uint8_t _enc28j60_readReg(ENC28J60 *enc28j60, uint8_t address);
uint8_t _enc28j60_readOp(ENC28J60 *enc28j60, uint8_t op, uint8_t address);
uint16_t _enc28j60_readOp16(ENC28J60 *enc28j60, uint8_t op, uint8_t address);
void _enc28j60_readBuffer(ENC28J60 *enc28j60, uint8_t *data, uint16_t len);
void _enc28j60_writeBuffer(ENC28J60 *enc28j60, uint8_t *data, uint16_t len);
void _enc28j60_debugDump(ENC28J60 *enc28j60);

void _enc28j60_csDeassert(ENC28J60 *enc28j60);
void _enc28j60_csAssert(ENC28J60 *enc28j60);
void _enc28j60_resetDeassert(ENC28J60 *enc28j60);
void _enc28j60_resetAssert(ENC28J60 *enc28j60);
uint8_t _enc28j60_spiTransfer(ENC28J60 *enc28j60, uint8_t d);

void enc28j60_setup(ENC28J60 *enc28j60) {
  GPIO_InitParams gpio;

  enc28j60->_enc28j60_bank = 0xff;

  RCC_peripheralClockEnableForPort(enc28j60->csPort);
  RCC_peripheralClockEnableForPort(enc28j60->resetPort);

  GPIO_initParamsInit(&gpio);
  gpio.mode = GPIO_Mode_output;
  gpio.outputType = GPIO_OutputType_pushPull;
  gpio.speed = GPIO_Speed_high;

  gpio.port = enc28j60->csPort;
  gpio.pin = enc28j60->csPin;
  GPIO_init(&gpio);
  _enc28j60_csDeassert(enc28j60);

  gpio.port = enc28j60->resetPort;
  gpio.pin = enc28j60->resetPin;
  GPIO_init(&gpio);

  timer_set(&enc28j60->_enc28j60_periodicTimer, CLOCK_SECOND / 4);

  // perform system reset
  _enc28j60_resetAssert(enc28j60);
  sleep_ms(50);
  _enc28j60_resetDeassert(enc28j60);
  sleep_ms(1000);

  _enc28j60_writeOp(enc28j60, ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
  sleep_ms(50);

  // check CLKRDY bit to see if reset is complete
  // The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
  while (1) {
    uint8_t r = _enc28j60_readReg(enc28j60, ESTAT);
    if (r & ESTAT_CLKRDY) {
      break;
    }
#ifdef ENC28J60_DEBUG
    printf("?ESTAT: 0x%02x\n", r);
#endif
    sleep_ms(100);
  }

  // do bank 0 stuff
  // initialize receive buffer
  // 16-bit transfers, must write low byte first
  // set receive buffer start address
  enc28j60->_enc28j60_nextPacketPtr = RXSTART_INIT;

  // Rx start
  _enc28j60_writeRegPair(enc28j60, ERXSTL, RXSTART_INIT);

  // set receive pointer address
  _enc28j60_writeRegPair(enc28j60, ERXRDPTL, RXSTART_INIT);

  // RX end
  _enc28j60_writeRegPair(enc28j60, ERXNDL, RXSTOP_INIT);

  // TX start
  _enc28j60_writeRegPair(enc28j60, ETXSTL, TXSTART_INIT);

  // TX end
  //_enc28j60_writeRegPair(enc28j60, ETXNDL, TXSTOP_INIT);

  // do bank 2 stuff
  // enable MAC receive
  // and bring MAC out of reset (writes 0x00 to MACON2)
  _enc28j60_writeRegPair(enc28j60, MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);

  // bring MAC out of reset
  _enc28j60_writeRegPair(enc28j60, MACON2, 0x00);

  // enable automatic padding to 60bytes and CRC operations
  _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);

  // set inter-frame gap (non-back-to-back)
  _enc28j60_writeRegPair(enc28j60, MAIPGL, 0x0C12);

  // set inter-frame gap (back-to-back)
  _enc28j60_writeReg(enc28j60, MABBIPG, 0x12);

  // Set the maximum packet size which the controller will accept
  // Do not send packets longer than MAX_FRAMELEN:
  _enc28j60_writeRegPair(enc28j60, MAMXFLL, MAX_FRAMELEN);

  // do bank 1 stuff, packet filter:
  // For broadcast packets we allow only ARP packtets
  // All other packets should be unicast only for our mac (MAADR)
  //
  // The pattern to match on is therefore
  // Type     ETH.DST
  // ARP      BROADCAST
  // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
  // in binary these poitions are:11 0000 0011 1111
  // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
  //TODO define specific pattern to receive dhcp-broadcast packages instead of setting ERFCON_BCEN!
  _enc28j60_writeReg(enc28j60, ERXFCON, 0);//ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN | ERXFCON_BCEN);
  _enc28j60_writeRegPair(enc28j60, EPMM0, 0x303f);
  _enc28j60_writeRegPair(enc28j60, EPMCSL, 0xf7f9);

  // do bank 3 stuff
  // write MAC address
  // NOTE: MAC address in ENC28J60 is byte-backward
  _enc28j60_writeReg(enc28j60, MAADR5, enc28j60->macAddress[0]);
  _enc28j60_writeReg(enc28j60, MAADR4, enc28j60->macAddress[1]);
  _enc28j60_writeReg(enc28j60, MAADR3, enc28j60->macAddress[2]);
  _enc28j60_writeReg(enc28j60, MAADR2, enc28j60->macAddress[3]);
  _enc28j60_writeReg(enc28j60, MAADR1, enc28j60->macAddress[4]);
  _enc28j60_writeReg(enc28j60, MAADR0, enc28j60->macAddress[5]);

  // no loopback of transmitted frames
  _enc28j60_phyWrite(enc28j60, PHCON2, PHCON2_HDLDIS);

  // switch to bank 0
  _enc28j60_setBank(enc28j60, ECON1);

  // enable interrutps
  _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);

  // enable packet reception
  _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

  // Configure leds
  _enc28j60_phyWrite(enc28j60, PHLCON, 0x476);

  _enc28j60_debugDump(enc28j60);

  tcpip_set_outputfunc(enc28j60_tcp_output);

  sleep_ms(100);
}

void _enc28j60_csDeassert(ENC28J60 *enc28j60) {
  GPIO_setBits(enc28j60->csPort, enc28j60->csPin);
}

void _enc28j60_csAssert(ENC28J60 *enc28j60) {
  GPIO_resetBits(enc28j60->csPort, enc28j60->csPin);
}

void _enc28j60_resetDeassert(ENC28J60 *enc28j60) {
  GPIO_setBits(enc28j60->resetPort, enc28j60->resetPin);
}

void _enc28j60_resetAssert(ENC28J60 *enc28j60) {
  GPIO_resetBits(enc28j60->resetPort, enc28j60->resetPin);
}

uint8_t _enc28j60_spiTransfer(ENC28J60 *enc28j60, uint8_t d) {
  return SPI_transfer(enc28j60->spi, d);
}

void _enc28j60_debugDump(ENC28J60 *enc28j60) {
  printf("?RevID: 0x%02x\n", _enc28j60_readReg(enc28j60, EREVID));

  printf("?Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\n");
  printf("?          %02x   %02x  %02x  %02x   %02x\n",
         _enc28j60_readReg(enc28j60, ECON1), _enc28j60_readReg(enc28j60, ECON2), _enc28j60_readReg(enc28j60, ESTAT), _enc28j60_readReg(enc28j60, EIR), _enc28j60_readReg(enc28j60, EIE));

  printf("?MAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\n");
  printf("?       0x%02x  0x%02x  0x%02x  0x%02x  %02x-%02x-%02x-%02x-%02x-%02x\n",
         _enc28j60_readReg(enc28j60, MACON1), _enc28j60_readReg(enc28j60, MACON2), _enc28j60_readReg(enc28j60, MACON3), _enc28j60_readReg(enc28j60, MACON4),
         _enc28j60_readReg(enc28j60, MAADR5), _enc28j60_readReg(enc28j60, MAADR4), _enc28j60_readReg(enc28j60, MAADR3), _enc28j60_readReg(enc28j60, MAADR2),
         _enc28j60_readReg(enc28j60, MAADR1), _enc28j60_readReg(enc28j60, MAADR0));

  printf("?Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\n");
  printf("?       0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x%02x 0x%02x 0x%02x 0x%02x%02x\n",
         _enc28j60_readReg(enc28j60, ERXSTH), _enc28j60_readReg(enc28j60, ERXSTL), _enc28j60_readReg(enc28j60, ERXNDH), _enc28j60_readReg(enc28j60, ERXNDL),
         _enc28j60_readReg(enc28j60, ERXWRPTH), _enc28j60_readReg(enc28j60, ERXWRPTL), _enc28j60_readReg(enc28j60, ERXRDPTH), _enc28j60_readReg(enc28j60, ERXRDPTL),
         _enc28j60_readReg(enc28j60, ERXFCON), _enc28j60_readReg(enc28j60, EPKTCNT), _enc28j60_readReg(enc28j60, MAMXFLH), _enc28j60_readReg(enc28j60, MAMXFLL));

  printf("?Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\n");
  printf("?       0x%02x%02x  0x%02x%02x  0x%02x   0x%02x  0x%02x\n",
         _enc28j60_readReg(enc28j60, ETXSTH), _enc28j60_readReg(enc28j60, ETXSTL), _enc28j60_readReg(enc28j60, ETXNDH), _enc28j60_readReg(enc28j60, ETXNDL),
         _enc28j60_readReg(enc28j60, MACLCON1), _enc28j60_readReg(enc28j60, MACLCON2), _enc28j60_readReg(enc28j60, MAPHSUP));
}

void enc28j60_tick(ENC28J60 *enc28j60) {
  uip_len = _enc28j60_receivePacket(enc28j60, ((uint8_t *)uip_buf), UIP_BUFSIZE);
  if (uip_len > 0) {
    struct uip_eth_hdr *header = ((struct uip_eth_hdr *)&uip_buf[0]);
    uint16_t packetType = header->type;

#ifdef ENC28J60_DEBUG
    printf("?receivePacket: len: %d, dest: %d.%d.%d.%d, src: %d.%d.%d.%d, type: %d\n",
           uip_len,
           header->dest.u8[0], header->dest.u8[1], header->dest.u8[2], header->dest.u8[3],
           header->src.u8[0], header->src.u8[1], header->src.u8[2], header->src.u8[3],
           packetType
          );
    for (int i = 0; i < uip_len; i++) {
      printf("%02x ", uip_buf[i]);
    }
    printf("\n");
#endif

    if (packetType == UIP_HTONS(UIP_ETHTYPE_IP)) {
#ifdef ENC28J60_DEBUG
      printf("?readPacket type IP\n");
#endif
      uip_arp_ipin();
      uip_input();
      if (uip_len > 0) {
        uip_arp_out();
        enc28j60_send(enc28j60);
      }
    } else if (packetType == UIP_HTONS(UIP_ETHTYPE_ARP)) {
#ifdef ENC28J60_DEBUG
      printf("?readPacket type ARP\n");
#endif
      uip_arp_arpin();
      if (uip_len > 0) {
        enc28j60_send(enc28j60);
      }
    }
  }

  if (timer_expired(&enc28j60->_enc28j60_periodicTimer)) {
    timer_restart(&enc28j60->_enc28j60_periodicTimer);
    for (int i = 0; i < UIP_CONNS; i++) {
      uip_periodic(i);
      // If the above function invocation resulted in data that
      // should be sent out on the Enc28J60Network, the global variable
      // uip_len is set to a value > 0.
      if (uip_len > 0) {
        uip_arp_out();
        enc28j60_send(enc28j60);
      }
    }

#if UIP_UDP
    for (int i = 0; i < UIP_UDP_CONNS; i++) {
      uip_udp_periodic(i);

      // If the above function invocation resulted in data that
      // should be sent out on the Enc28J60Network, the global variable
      // uip_len is set to a value > 0. */
      if (uip_len > 0) {
        uip_arp_out();
        enc28j60_send(enc28j60);
      }
    }
#endif /* UIP_UDP */
  }
}

uint16_t _enc28j60_receivePacket(ENC28J60 *enc28j60, uint8_t *buffer, uint16_t bufferLength) {
#ifdef ENC28J60_DEBUG
  uint16_t rxstat;
#endif
  uint16_t len;

  // check if a packet has been received and buffered
  //if( !(_enc28j60_readReg(EIR) & EIR_PKTIF) ){
  // The above does not work. See Rev. B4 Silicon Errata point 6.
  if (_enc28j60_readReg(enc28j60, EPKTCNT) == 0) {
    return 0;
  }

#ifdef ENC28J60_DEBUG
  printf("read from: 0x%04x\n", enc28j60->_enc28j60_nextPacketPtr);
#endif

  // Set the read pointer to the start of the received packet
  _enc28j60_writeRegPair(enc28j60, ERDPTL, enc28j60->_enc28j60_nextPacketPtr);

  // read the next packet pointer
  enc28j60->_enc28j60_nextPacketPtr = _enc28j60_readOp16(enc28j60, ENC28J60_READ_BUF_MEM, 0);
#ifdef ENC28J60_DEBUG
  printf("nextPacketPtr: 0x%04x\n", _enc28j60_nextPacketPtr);
#endif

  // read the packet length (see datasheet page 43)
  len = _enc28j60_readOp16(enc28j60, ENC28J60_READ_BUF_MEM, 0);
  len -= 4; //remove the CRC count

  len = min(len, bufferLength);

  // read the receive status (see datasheet page 43)
#ifdef ENC28J60_DEBUG
  rxstat = _enc28j60_readOp16(enc28j60, ENC28J60_READ_BUF_MEM, 0);
  printf("rxstat: 0x%04x\n", rxstat);
#else
  _enc28j60_readOp16(enc28j60, ENC28J60_READ_BUF_MEM, 0);
#endif

  _enc28j60_readBuffer(enc28j60, buffer, len);

  // Move the RX read pointer to the start of the next received packet
  // This frees the memory we just read out
  _enc28j60_writeRegPair(enc28j60, ERXRDPTL, enc28j60->_enc28j60_nextPacketPtr);

  // decrement the packet counter indicate we are done with this packet
  _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);

  return len;
}

void enc28j60_send(ENC28J60 *enc28j60) {
  if (uip_len <= UIP_LLH_LEN + 40) {
    _enc28j60_packetSend(enc28j60, (uint8_t *)uip_buf, uip_len, 0, 0);
  } else {
    _enc28j60_packetSend(enc28j60, (uint8_t *)uip_buf, 54, (uint8_t *)uip_appdata, uip_len - UIP_LLH_LEN - 40);
  }
}

void _enc28j60_writeReg(ENC28J60 *enc28j60, uint8_t address, uint8_t data) {
  // set the bank
  _enc28j60_setBank(enc28j60, address);

  // do the write
  _enc28j60_writeOp(enc28j60, ENC28J60_WRITE_CTRL_REG, address, data);
}

void _enc28j60_writeRegPair(ENC28J60 *enc28j60, uint8_t address, uint16_t data) {
  // set the bank
  _enc28j60_setBank(enc28j60, address);

  // do the write
  _enc28j60_writeOp(enc28j60, ENC28J60_WRITE_CTRL_REG, address, (data & 0xFF));
  _enc28j60_writeOp(enc28j60, ENC28J60_WRITE_CTRL_REG, address + 1, (data) >> 8);
}

void _enc28j60_phyWrite(ENC28J60 *enc28j60, uint8_t address, uint16_t data) {
  // set the PHY register address
  _enc28j60_writeReg(enc28j60, MIREGADR, address);

  // write the PHY data
  _enc28j60_writeRegPair(enc28j60, MIWRL, data);

  // wait until the PHY write completes
  while (_enc28j60_readReg(enc28j60, MISTAT) & MISTAT_BUSY) {
    sleep_us(15);
  }
}

uint16_t _enc28j60_phyRead(ENC28J60 *enc28j60, uint8_t address) {
  uint16_t data;

  // Set the right address and start the register read operation
  _enc28j60_writeReg(enc28j60, MIREGADR, address);
  _enc28j60_writeReg(enc28j60, MICMD, MICMD_MIIRD);

  // wait until the PHY read completes
  while (_enc28j60_readReg(enc28j60, MISTAT) & MISTAT_BUSY);

  // quit reading
  _enc28j60_writeReg(enc28j60, MICMD, 0x00);

  // get data value
  data = _enc28j60_readReg(enc28j60, MIRDL);
  data |= ((uint16_t)_enc28j60_readReg(enc28j60, MIRDH)) << 8;
  return data;
}

void _enc28j60_setBank(ENC28J60 *enc28j60, uint8_t address) {
  // set the bank (if needed)
  if ((address & BANK_MASK) != enc28j60->_enc28j60_bank) {
    _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_BSEL1 | ECON1_BSEL0);
    _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK) >> 5);
    enc28j60->_enc28j60_bank = (address & BANK_MASK);
  }
}

uint8_t _enc28j60_readReg(ENC28J60 *enc28j60, uint8_t address) {
  // set the bank
  _enc28j60_setBank(enc28j60, address);

  // do the read
  return _enc28j60_readOp(enc28j60, ENC28J60_READ_CTRL_REG, address);
}

void _enc28j60_packetSend(ENC28J60 *enc28j60, uint8_t *packet1, uint16_t packet1Length, uint8_t *packet2, uint16_t packet2Length) {
#ifdef ENC28J60_DEBUG
  int i;
  debug_write_line("packetSend:");
  debug_write("  packet1 ");
  debug_write_u16(packet1Length, 10);
  debug_write(": ");
  for (i = 0; i < packet1Length; i++) {
    debug_write_u8(packet1[i], 16);
    debug_write(" ");
  }
  debug_write_line("");
  debug_write("  packet2 ");
  debug_write_u16(packet2Length, 10);
  debug_write(": ");
  for (i = 0; i < packet2Length; i++) {
    debug_write_u8(packet2[i], 16);
    debug_write(" ");
  }
  debug_write_line("");
#endif

  // Errata: Transmit Logic reset
  _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRST);
  _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRST);

  // Set the write pointer to start of transmit buffer area
  _enc28j60_writeReg(enc28j60, EWRPTL, TXSTART_INIT & 0xff);
  _enc28j60_writeReg(enc28j60, EWRPTH, TXSTART_INIT >> 8);

  // Set the TXND pointer to correspond to the packet size given
  _enc28j60_writeReg(enc28j60, ETXNDL, (TXSTART_INIT + packet1Length + packet2Length));
  _enc28j60_writeReg(enc28j60, ETXNDH, (TXSTART_INIT + packet1Length + packet2Length) >> 8);

  // write per-packet control byte
  _enc28j60_writeOp(enc28j60, ENC28J60_WRITE_BUF_MEM, 0, 0x00);

  // copy the packet into the transmit buffer
  _enc28j60_writeBuffer(enc28j60, packet1, packet1Length);
  if (packet2Length > 0) {
    _enc28j60_writeBuffer(enc28j60, packet2, packet2Length);
  }

  // send the contents of the transmit buffer onto the network
  _enc28j60_writeOp(enc28j60, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
}

uint8_t _enc28j60_readOp(ENC28J60 *enc28j60, uint8_t op, uint8_t address) {
  uint8_t  result;

  _enc28j60_csAssert(enc28j60);

  _enc28j60_spiTransfer(enc28j60, op | (address & ADDR_MASK)); // issue read command
  result = _enc28j60_spiTransfer(enc28j60, 0x00);

  // do dummy read if needed (for mac and mii, see datasheet page 29)
  if (address & 0x80) {
    result = _enc28j60_spiTransfer(enc28j60, 0x00);
  }

  _enc28j60_csDeassert(enc28j60);
  return result;
}

uint16_t _enc28j60_readOp16(ENC28J60 *enc28j60, uint8_t op, uint8_t address) {
  uint16_t result;
  result = _enc28j60_readOp(enc28j60, op, address);
  result |= ((uint16_t)_enc28j60_readOp(enc28j60, op, address)) << 8;
  return result;
}

void _enc28j60_writeOp(ENC28J60 *enc28j60, uint8_t op, uint8_t address, uint8_t data) {
  _enc28j60_csAssert(enc28j60);
  _enc28j60_spiTransfer(enc28j60, op | (address & ADDR_MASK));
  _enc28j60_spiTransfer(enc28j60, data);
  _enc28j60_csDeassert(enc28j60);
}

void _enc28j60_readBuffer(ENC28J60 *enc28j60, uint8_t *data, uint16_t len) {
  _enc28j60_csAssert(enc28j60);

  _enc28j60_spiTransfer(enc28j60, ENC28J60_READ_BUF_MEM);
  while (len--) {
    *data++ = _enc28j60_spiTransfer(enc28j60, 0x00);
  }

  _enc28j60_csDeassert(enc28j60);
}

void _enc28j60_writeBuffer(ENC28J60 *enc28j60, uint8_t *data, uint16_t len) {
  _enc28j60_csAssert(enc28j60);

  _enc28j60_spiTransfer(enc28j60, ENC28J60_WRITE_BUF_MEM);
  while (len--) {
    _enc28j60_spiTransfer(enc28j60, *data++);
  }

  _enc28j60_csDeassert(enc28j60);
}

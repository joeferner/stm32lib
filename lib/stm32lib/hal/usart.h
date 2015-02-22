
#ifndef _STM32LIB_HAL_USART_H_
#define _STM32LIB_HAL_USART_H_

#include "base.h"

typedef USART_TypeDef *USART_Instance;
#define IS_USART(i) ( \
    (i == USART1) || \
    (i == USART2) || \
    (i == USART3) || \
    (i == USART4)  \
  )

typedef enum {
  USART_WordLength_8b = ((0 << 28) || (0 << 12)),
  USART_WordLength_9b = ((0 << 28) || (1 << 12)),
  USART_WordLength_7b = ((1 << 28) || (0 << 12))
} USART_WordLength;
#define IS_USART_WORD_LENGTH(v) ( \
    (v == USART_WordLength_8b) || \
    (v == USART_WordLength_9b) || \
    (v == USART_WordLength_7b) \
  )
#define USART_WORD_LENGTH_mask (USART_WordLength_8b | USART_WordLength_9b | USART_WordLength_7b)

typedef enum {
  USART_Parity_no = ((0 << 10) || (0 << 9)),
  USART_Parity_even = ((1 << 10) || (0 << 9)),
  USART_Parity_odd = ((1 << 10) || (1 << 9))
} USART_Parity;
#define IS_USART_PARITY(v) ( \
    (v == USART_Parity_no) || \
    (v == USART_Parity_even) || \
    (v == USART_Parity_odd) \
  )
#define USART_PARITY_mask (USART_Parity_no | USART_Parity_even | USART_Parity_odd)

typedef enum {
  USART_StopBits_1 = (0b00 << 12),
  USART_StopBits_2 = (0b10 << 12),
  USART_StopBits_1_5 = (0b11 << 12)
} USART_StopBits;
#define IS_USART_STOP_BITS(v) ( \
    (v == USART_StopBits_1) || \
    (v == USART_StopBits_2) || \
    (v == USART_StopBits_1_5) \
  )
#define USART_STOP_BITS_mask (USART_StopBits_1 | USART_StopBits_2 | USART_StopBits_1_5)

typedef enum {
  USART_HardwareFlowControl_none = (0b00 << 8),
  USART_HardwareFlowControl_rts = (0b01 << 8),
  USART_HardwareFlowControl_cts = (0b10 << 8),
  USART_HardwareFlowControl_rts_cts = (0b11 << 8)
} USART_HardwareFlowControl;
#define IS_USART_HARDWARE_FLOW_CONTROL(v) ( \
    (v == USART_HardwareFlowControl_none) || \
    (v == USART_HardwareFlowControl_rts) || \
    (v == USART_HardwareFlowControl_cts) || \
    (v == USART_HardwareFlowControl_rts_cts) \
  )
#define USART_HARDWARE_FLOW_CONTROL_MASK (USART_HardwareFlowControl_none | USART_HardwareFlowControl_rts | USART_HardwareFlowControl_cts | USART_HardwareFlowControl_rts_cts)

typedef enum {
  USART_Mode_tx = (1 << 3),
  USART_Mode_rx = (1 << 2)
} USART_Mode;
#define IS_USART_MODE(v) ( \
    (v == USART_Mode_tx) || \
    (v == USART_Mode_rx) || \
    (v == (USART_Mode_rx | USART_Mode_tx)) \
  )
#define USART_MODE_mask (USART_Mode_tx | USART_Mode_rx)

typedef enum {
  USART_Flag_PE    = 0x00000001,
  USART_Flag_FE    = 0x00000002,
  USART_Flag_NE    = 0x00000004,
  USART_Flag_ORE   = 0x00000008,
  USART_Flag_IDLE  = 0x00000010,
  USART_Flag_RXNE  = 0x00000020,
  USART_Flag_TC    = 0x00000040,
  USART_Flag_TXE   = 0x00000080,
  USART_Flag_LBDF  = 0x00000100,
  USART_Flag_CTSIF = 0x00000200,
  USART_Flag_CTS   = 0x00000400,
  USART_Flag_RTOF  = 0x00000800,
  USART_Flag_EOBF  = 0x00001000,
  USART_Flag_ABRE  = 0x00004000,
  USART_Flag_ABRF  = 0x00008000,
  USART_Flag_BUSY  = 0x00010000,
  USART_Flag_CMF   = 0x00020000,
  USART_Flag_SBKF  = 0x00040000,
  USART_Flag_RWU   = 0x00080000,
  USART_Flag_WUF   = 0x00100000,
  USART_Flag_TEACK = 0x00200000,
  USART_Flag_REACK = 0x00400000,
} USART_Flag;
#define IS_USART_FLAG(v) ( \
  ((v) == USART_Flag_PE) \
  || ((v) == USART_Flag_FE) \
  || ((v) == USART_Flag_NE) \
  || ((v) == USART_Flag_ORE) \
  || ((v) == USART_Flag_IDLE) \
  || ((v) == USART_Flag_RXNE) \
  || ((v) == USART_Flag_TC) \
  || ((v) == USART_Flag_TXE) \
  || ((v) == USART_Flag_LBDF) \
  || ((v) == USART_Flag_CTSIF) \
  || ((v) == USART_Flag_CTS) \
  || ((v) == USART_Flag_RTOF) \
  || ((v) == USART_Flag_EOBF) \
  || ((v) == USART_Flag_ABRE) \
  || ((v) == USART_Flag_ABRF) \
  || ((v) == USART_Flag_BUSY) \
  || ((v) == USART_Flag_CMF) \
  || ((v) == USART_Flag_SBKF) \
  || ((v) == USART_Flag_RWU) \
  || ((v) == USART_Flag_WUF) \
  || ((v) == USART_Flag_TEACK) \
  || ((v) == USART_Flag_REACK) \
)

#define IS_USART_DATA(d)            ((d) <= 0x1FF)
#define IS_USART_BAUDRATE(b)        (((b) > 0) && ((b) < 0x0044AA21))

typedef struct {
  USART_TypeDef *instance;
  uint32_t baudRate;
  USART_WordLength wordLength;
  USART_Parity parity;
  USART_StopBits stopBits;
  USART_HardwareFlowControl hardwareFlowControl;
  USART_Mode mode;
} HAL_USART_InitParams;

void HAL_USART_initParamsInit(HAL_USART_InitParams *initParams);
void HAL_USART_init(HAL_USART_InitParams *initParams);
void USART_enable(USART_Instance instance);
void USART_disable(USART_Instance instance);
void USART_setState(USART_Instance instance, FunctionalState state);
bool USART_rxHasData(USART_Instance instance);
uint16_t USART_rx(USART_Instance instance);
void USART_tx(USART_Instance instance, uint8_t b);
void USART_txWaitForComplete(USART_Instance instance);
bool USART_txComplete(USART_Instance instance);
FlagStatus USART_getFlagStatus(USART_Instance instance, USART_Flag flag);
void USART_clearFlag(USART_Instance instance, USART_Flag flag);
void USART_interruptTransmissionComplete(USART_Instance instance, FunctionalState state);
void USART_interruptReceive(USART_Instance instance, FunctionalState state);
void USART_interruptsEnable(USART_Instance instance, uint32_t priority);

#endif

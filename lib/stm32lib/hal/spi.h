
#ifndef _STM32LIB_HAL_SPI_H_
#define _STM32LIB_HAL_SPI_H_

#include "base.h"

typedef SPI_TypeDef *SPI_Instance;

#define IS_SPI_INSTANCE(s) ( \
  ((s) == SPI1) \
)

typedef enum {
  SPI_Direction_2LinesFullDuplex = 0x0000,
  SPI_Direction_2LinesRxOnly = 0x0400,
  SPI_Direction_1LineRx = 0x8000,
  SPI_Direction_1LineTx = 0xC000
} SPI_Direction;
#define SPI_Direction_mask 0xC400
#define IS_SPI_DIRECTION(m) ( \
  ((m) == SPI_Direction_2LinesFullDuplex) \
  || ((m) == SPI_Direction_2LinesRxOnly) \
  || ((m) == SPI_Direction_1LineRx) \
  || ((m) == SPI_Direction_1LineTx) \
  )

typedef enum {
  SPI_Mode_master = 0x0104,
  SPI_Mode_slave = 0x0000
} SPI_Mode;
#define SPI_Mode_mask 0x0104
#define IS_SPI_MODE(m) ( \
  ((m) == SPI_Mode_master) \
  || ((m) == SPI_Mode_slave) \
)

typedef enum {
  SPI_DataSize_4b = 0x300,
  SPI_DataSize_5b = 0x400,
  SPI_DataSize_6b = 0x500,
  SPI_DataSize_7b = 0x600,
  SPI_DataSize_8b = 0x700,
  SPI_DataSize_9b = 0x800,
  SPI_DataSize_10b = 0x900,
  SPI_DataSize_11b = 0xa00,
  SPI_DataSize_12b = 0xb00,
  SPI_DataSize_13b = 0xc00,
  SPI_DataSize_14b = 0xd00,
  SPI_DataSize_15b = 0xe00,
  SPI_DataSize_16b = 0xf00
} SPI_DataSize;
#define SPI_DataSize_mask 0x0f00
#define IS_SPI_DATASIZE(d) ( \
  ((d) == SPI_DataSize_4b) \
  || ((d) == SPI_DataSize_5b) \
  || ((d) == SPI_DataSize_6b) \
  || ((d) == SPI_DataSize_7b) \
  || ((d) == SPI_DataSize_8b) \
  || ((d) == SPI_DataSize_9b) \
  || ((d) == SPI_DataSize_10b) \
  || ((d) == SPI_DataSize_11b) \
  || ((d) == SPI_DataSize_12b) \
  || ((d) == SPI_DataSize_13b) \
  || ((d) == SPI_DataSize_14b) \
  || ((d) == SPI_DataSize_15b) \
  || ((d) == SPI_DataSize_16b) \
)
#define IS_SPI_DATASIZE_8BIT_OR_LESS(d) ( \
  ((d) == SPI_DataSize_4b) \
  || ((d) == SPI_DataSize_5b) \
  || ((d) == SPI_DataSize_6b) \
  || ((d) == SPI_DataSize_7b) \
  || ((d) == SPI_DataSize_8b) \
)

typedef enum {
  SPI_NSS_soft = 0x0200,
  SPI_NSS_hard = 0x0000
} SPI_NSS;
#define SPI_NSS_mask 0x0200
#define IS_SPI_NSS(d) ( \
  ((d) == SPI_NSS_soft) \
  || ((d) == SPI_NSS_hard) \
)

typedef enum {
  SPI_BaudRatePrescaler_2 = 0x0000,
  SPI_BaudRatePrescaler_4 = 0x0008,
  SPI_BaudRatePrescaler_8 = 0x0010,
  SPI_BaudRatePrescaler_16 = 0x0018,
  SPI_BaudRatePrescaler_32 = 0x0020,
  SPI_BaudRatePrescaler_64 = 0x0028,
  SPI_BaudRatePrescaler_128 = 0x0030,
  SPI_BaudRatePrescaler_256 = 0x0038
} SPI_BaudRatePrescaler;
#define SPI_BaudRatePrescaler_mask 0x38
#define IS_SPI_BAUD_RATE_PRESCALER(d) ( \
  ((d) == SPI_BaudRatePrescaler_2) \
  || ((d) == SPI_BaudRatePrescaler_4) \
  || ((d) == SPI_BaudRatePrescaler_8) \
  || ((d) == SPI_BaudRatePrescaler_16) \
  || ((d) == SPI_BaudRatePrescaler_32) \
  || ((d) == SPI_BaudRatePrescaler_64) \
  || ((d) == SPI_BaudRatePrescaler_128) \
  || ((d) == SPI_BaudRatePrescaler_256) \
)

typedef enum {
  SPI_FirstBit_msb = 0x00,
  SPI_FirstBit_lsb = 0x80
} SPI_FirstBit;
#define SPI_FirstBit_mask 0x80
#define IS_SPI_FIRST_BIT(d) ( \
  ((d) == SPI_FirstBit_msb) \
  || ((d) == SPI_FirstBit_lsb) \
)

/**
 * +------+------+-------+
 * | Mode | CPOL | CPHA  |
 * +------+------+-------+
 * |  0   | low  | 1Edge |
 * |  1   | low  | 2Edge |
 * |  2   | high | 1Edge |
 * |  3   | high | 2Edge |
 * +------+------+-------+
 */
typedef enum {
  SPI_CPOL_low = 0x00,
  SPI_CPOL_high = 0x02
} SPI_CPOL;
#define SPI_CPOL_mask 0x02
#define IS_SPI_CPOL(d) ( \
  ((d) == SPI_CPOL_low) \
  || ((d) == SPI_CPOL_high) \
)

typedef enum {
  SPI_CPHA_1Edge = 0x00,
  SPI_CPHA_2Edge = 0x01
} SPI_CPHA;
#define SPI_CPHA_mask 0x01
#define IS_SPI_CPHA(d) ( \
  ((d) == SPI_CPHA_1Edge) \
  || ((d) == SPI_CPHA_2Edge) \
)

typedef enum {
  SPI_Flag_RXNE = 0x0001,
  SPI_Flag_TXE = 0x0002,
  SPI_Flag_BSY = 0x0080
} SPI_Flag;
#define IS_SPI_FLAG(d) ( \
  ((d) == SPI_Flag_RXNE) \
  || ((d) == SPI_Flag_TXE) \
  || ((d) == SPI_Flag_BSY) \
)

typedef struct {
  SPI_Instance instance;
  SPI_Direction direction;
  SPI_Mode mode;
  SPI_DataSize dataSize;
  SPI_NSS nss;
  SPI_BaudRatePrescaler baudRatePrescaler;
  SPI_FirstBit firstBit;
  SPI_CPOL cpol;
  SPI_CPHA cpha;
} HAL_SPI_InitParams;

void HAL_SPI_initParamsInit(HAL_SPI_InitParams *initParams);
void HAL_SPI_init(HAL_SPI_InitParams *initParams);
void SPI_enable(SPI_Instance instance);
void SPI_disable(SPI_Instance instance);
void SPI_setState(SPI_Instance instance, FunctionalState state);
void SPI_sendData(SPI_Instance instance, uint8_t d);
uint8_t SPI_receiveData(SPI_Instance instance);
FlagStatus SPI_getFlagStatus(SPI_Instance instance, SPI_Flag flag);

#endif


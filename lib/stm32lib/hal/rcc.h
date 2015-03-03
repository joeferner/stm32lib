
#ifndef _STM32LIB_HAL_RCC_H_
#define _STM32LIB_HAL_RCC_H_

#include "base.h"

typedef enum  {
  RCC_Peripheral_AFIO   = 0x0000001,
  RCC_Peripheral_SYSCFG = 0x0000002,
  RCC_Peripheral_GPIOA  = 0x0000004,
  RCC_Peripheral_GPIOB  = 0x0000008,
  RCC_Peripheral_GPIOC  = 0x0000010,
  RCC_Peripheral_GPIOD  = 0x0000020,
  RCC_Peripheral_GPIOE  = 0x0000040,
  RCC_Peripheral_GPIOF  = 0x0000080,
  RCC_Peripheral_DAC    = 0x0000100,
  RCC_Peripheral_USART1 = 0x0000200,
  RCC_Peripheral_SPI1   = 0x0000400,
  RCC_Peripheral_SPI2   = 0x0000800
} RCC_Peripheral;
#define IS_RCC_PERIPHERAL(v) ( \
  ((v) == RCC_Peripheral_AFIO) \
  || ((v) == RCC_Peripheral_SYSCFG) \
  || ((v) == RCC_Peripheral_GPIOA) \
  || ((v) == RCC_Peripheral_GPIOB) \
  || ((v) == RCC_Peripheral_GPIOC) \
  || ((v) == RCC_Peripheral_GPIOD) \
  || ((v) == RCC_Peripheral_GPIOE) \
  || ((v) == RCC_Peripheral_GPIOF) \
  || ((v) == RCC_Peripheral_DAC) \
  || ((v) == RCC_Peripheral_USART1) \
  || ((v) == RCC_Peripheral_SPI1) \
  || ((v) == RCC_Peripheral_SPI2) \
)

typedef struct {
  uint32_t SYSCLK_Frequency;
  uint32_t HCLK_Frequency;
  uint32_t PCLK_Frequency;
  uint32_t ADCCLK_Frequency;
  uint32_t CECCLK_Frequency;
  uint32_t I2C1CLK_Frequency;
  uint32_t USART1CLK_Frequency;
  uint32_t USART2CLK_Frequency; /*!< Only applicable for STM32F072 and STM32F091 devices */
  uint32_t USART3CLK_Frequency; /*!< Only applicable for STM32F091 devices */
  uint32_t USBCLK_Frequency;    /*!< Only applicable for STM32F072 devices */
} RCC_Clocks;

void RCC_peripheralClockEnable(RCC_Peripheral set);
void RCC_peripheralClock(RCC_Peripheral set, FunctionalState state);
void RCC_getClocks(RCC_Clocks *clocks);

#endif

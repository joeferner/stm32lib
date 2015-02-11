
#include "rcc.h"

#define RCC_CFGR_HPRE_BITNUMBER    4
#define RCC_CFGR_PPRE_BITNUMBER    8
#define RCC_CFGR_PLLMUL_BITNUMBER  18
#define RCC_CFGR2_PREDIV_BITNUMBER 0

/** @defgroup RCC_System_Clock_Source_Status RCC System Clock Source Status
  * @{
  */
#define RCC_SYSCLKSOURCE_STATUS_HSI      RCC_CFGR_SWS_HSI
#define RCC_SYSCLKSOURCE_STATUS_HSE      RCC_CFGR_SWS_HSE
#define RCC_SYSCLKSOURCE_STATUS_PLLCLK   RCC_CFGR_SWS_PLL
#define RCC_SYSCLKSOURCE_STATUS_HSI48    RCC_CFGR_SWS_HSI48
/**
  * @}
  */

/** @defgroup RCC_PLL_Clock_Source RCC PLL Clock Source
  * @{
  */
#define RCC_PLLSOURCE_HSE                RCC_CFGR_PLLSRC_HSE_PREDIV
/**
  * @}
  */

/** @defgroup RCCEx_PLL_Clock_Source RCCEx PLL Clock Source
  * @{
  */
#define RCC_PLLSOURCE_HSI                RCC_CFGR_PLLSRC_HSI_PREDIV
#define RCC_PLLSOURCE_HSI48              RCC_CFGR_PLLSRC_HSI48_PREDIV

#define IS_RCC_PLLSOURCE(SOURCE) (((SOURCE) == RCC_PLLSOURCE_HSI)   || \
                                  ((SOURCE) == RCC_PLLSOURCE_HSI48) || \
                                  ((SOURCE) == RCC_PLLSOURCE_HSE))
/**
  * @}
  */

/** @defgroup RCCEx_Private_Variables RCCEx Private Variables
  * @{
  */
const uint8_t APBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t PLLMULFactorTable[16] = { 2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 16};
const uint8_t PredivFactorTable[16] = { 1, 2,  3,  4,  5,  6,  7,  8, 9, 10, 11, 12, 13, 14, 15, 16};
/**
  * @}
  */

void RCC_peripheralClockEnable(uint64_t set) {
  RCC_peripheralClock(set, ENABLE);
}

void RCC_peripheralClock(uint64_t set, FunctionalState state) {
  uint32_t ahbenr = 0;
  uint32_t apb1enr = 0;
  uint32_t apb2enr = 0;

  assert_param(IS_RCC_PERIPHERAL(set));
  assert_param(IS_FUNCTIONAL_STATE(state));

  if (set & RCC_peripheral_AFIO)   {}
  if (set & RCC_peripheral_GPIOA)  ahbenr |= (1 << 17);
  if (set & RCC_peripheral_USART1) apb2enr |= (1 << 14);

  if (state != DISABLE) {
    RCC->AHBENR |= ahbenr;
    RCC->APB1ENR |= apb1enr;
    RCC->APB2ENR |= apb2enr;
  } else {
    RCC->AHBENR &= ~ahbenr;
    RCC->APB1ENR &= ~apb1enr;
    RCC->APB2ENR &= ~apb2enr;
  }
}

/**
  * @brief  Returns the PCLK1 frequency
  * @note   Each time PCLK1 changes, this function must be called to update the
  *         right PCLK1 value. Otherwise, any configuration based on this function will be incorrect.
  * @retval PCLK1 frequency
  */
uint32_t RCC_getPCLK1Freq() {
  /* Get HCLK source and Compute PCLK1 frequency ---------------------------*/
  return (RCC_getHCLKFreq() >> APBAHBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE) >> RCC_CFGR_PPRE_BITNUMBER]);
}

/**
  * @brief  Returns the HCLK frequency
  * @note   Each time HCLK changes, this function must be called to update the
  *         right HCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency
  *         and updated within this function
  *
  * @retval HCLK frequency
  */
uint32_t RCC_getHCLKFreq() {
  SystemCoreClock = RCC_getSysClockFreq() >> APBAHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_BITNUMBER];
  return SystemCoreClock;
}

/**
  * @brief  Returns the SYSCLK frequency
  * @note   The system frequency computed by this function is not the real
  *         frequency in the chip. It is calculated based on the predefined
  *         constant and the selected clock source:
  * @note     If SYSCLK source is HSI, function returns a value based on HSI_VALUE(*)
  * @note     If SYSCLK source is HSI48, function returns a value based on HSI48_VALUE(*)
  * @note     If SYSCLK source is HSE, function returns a value based on HSE_VALUE
  *           divided by PREDIV factor(**)
  * @note     If SYSCLK source is PLL, function returns a value based on HSE_VALUE
  *           divided by PREDIV factor(**) or depending on STM32F0xx devices either a value based
  *           on HSI_VALUE divided by 2 or HSI_VALUE divided by PREDIV factor(*) multiplied by the
  *           PLL factor .
  * @note     (*) HSI_VALUE & HSI48_VALUE are constants defined in stm32f0xx_hal_conf.h file
  *               (default values 8 MHz and 48MHz).
  * @note     (**) HSE_VALUE is a constant defined in stm32f0xx_hal_conf.h file (default value
  *                8 MHz), user has to ensure that HSE_VALUE is same as the real
  *                frequency of the crystal used. Otherwise, this function may
  *                have wrong result.
  *
  * @note   The result of this function could be not correct when using fractional
  *         value for HSE crystal.
  *
  * @note   This function can be used by the user application to compute the
  *         baudrate for the communication peripherals or configure other parameters.
  *
  * @note   Each time SYSCLK changes, this function must be called to update the
  *         right SYSCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  * @retval SYSCLK frequency
  */
uint32_t RCC_getSysClockFreq() {
  uint32_t tmpreg = 0, prediv = 0, pllmul = 0, pllclk = 0;
  uint32_t sysclockfreq = 0;

  tmpreg = RCC->CFGR;

  /* Get SYSCLK source -------------------------------------------------------*/
  switch (tmpreg & RCC_CFGR_SWS) {
  case RCC_SYSCLKSOURCE_STATUS_HSE:    /* HSE used as system clock  source */
    sysclockfreq = HSE_VALUE;
    break;

  case RCC_SYSCLKSOURCE_STATUS_PLLCLK: /* PLL used as system clock  source */
    pllmul = PLLMULFactorTable[(uint32_t)(tmpreg & RCC_CFGR_PLLMUL) >> RCC_CFGR_PLLMUL_BITNUMBER];
    prediv = PredivFactorTable[(uint32_t)(RCC->CFGR2 & RCC_CFGR2_PREDIV) >> RCC_CFGR2_PREDIV_BITNUMBER];
    if ((tmpreg & RCC_CFGR_PLLSRC) == RCC_PLLSOURCE_HSE) {
      /* HSE used as PLL clock source : PLLCLK = HSE/PREDIV * PLLMUL */
      pllclk = (HSE_VALUE / prediv) * pllmul;
    }
#if defined(STM32F042x6) || defined(STM32F048xx) || defined(STM32F072xB) || defined(STM32F078xx) || \
    defined(STM32F091xC) || defined(STM32F098xx)
    else if ((tmpreg & RCC_CFGR_PLLSRC) == RCC_PLLSOURCE_HSI48) {
      /* HSI48 used as PLL clock source : PLLCLK = HSI48/PREDIV * PLLMUL */
      pllclk = (HSI48_VALUE / prediv) * pllmul;
    }
#endif /* STM32F042x6 || STM32F048xx || STM32F072xB || STM32F078xx || */
    /* STM32F091xC || STM32F098xx */
    else {
#if defined(STM32F042x6) || defined(STM32F048xx) || defined(STM32F070x6) || \
    defined(STM32F071xB) || defined(STM32F072xB) || defined(STM32F078xx) || defined(STM32F070xB) || \
    defined(STM32F091xC) || defined(STM32F098xx) || defined(STM32F030xC)
      /* HSI used as PLL clock source : PLLCLK = HSI/PREDIV * PLLMUL */
      pllclk = (HSI_VALUE / prediv) * pllmul;
#else
      /* HSI used as PLL clock source : PLLCLK = HSI/2 * PLLMUL */
      pllclk = (HSI_VALUE >> 1) * pllmul;
#endif /* STM32F042x6 || STM32F048xx || STM32F070x6 || 
          STM32F071xB || STM32F072xB || STM32F078xx || STM32F070xB
          STM32F091xC || STM32F098xx || STM32F030xC */
    }
    sysclockfreq = pllclk;
    break;

#if defined(STM32F042x6) || defined(STM32F048xx) || defined(STM32F072xB) || defined(STM32F078xx) || \
    defined(STM32F091xC) || defined(STM32F098xx)
  case RCC_SYSCLKSOURCE_STATUS_HSI48:    /* HSI48 used as system clock source */
    sysclockfreq = HSI48_VALUE;
    break;
#endif /* STM32F042x6 || STM32F048xx || STM32F072xB || STM32F078xx || */
    /* STM32F091xC || STM32F098xx */

  case RCC_SYSCLKSOURCE_STATUS_HSI:    /* HSI used as system clock source */
  default:
    sysclockfreq = HSI_VALUE;
    break;
  }
  return sysclockfreq;
}


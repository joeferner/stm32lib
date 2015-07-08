
#include "base.h"
#include "rcc.h"

#ifdef STM32F0XX
static volatile uint8_t APBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
#endif

#ifdef STM32F10X
#define CFGR_PLLMull_Mask         ((uint32_t)0x003C0000)
#define CFGR_PLLSRC_Mask          ((uint32_t)0x00010000)
#define CFGR_PLLXTPRE_Mask        ((uint32_t)0x00020000)
#define CFGR_SWS_Mask             ((uint32_t)0x0000000C)
#define CFGR_HPRE_Set_Mask        ((uint32_t)0x000000F0)
#define CFGR_PPRE1_Set_Mask       ((uint32_t)0x00000700)
#define CFGR_PPRE2_Set_Mask       ((uint32_t)0x00003800)
#define CFGR_ADCPRE_Set_Mask      ((uint32_t)0x0000C000)

static __I uint8_t APBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};
static __I uint8_t ADCPrescTable[4] = {2, 4, 6, 8};
#endif

void RCC_peripheralClockEnable(RCC_Peripheral set) {
  RCC_peripheralClock(set, ENABLE);
}

void RCC_peripheralClock(RCC_Peripheral set, FunctionalState state) {
  uint32_t ahbenr = 0;
  uint32_t apb1enr = 0;
  uint32_t apb2enr = 0;

  assert_param(IS_RCC_PERIPHERAL(set));
  assert_param(IS_FUNCTIONAL_STATE(state));

#ifdef STM32F0XX
  if (set & RCC_Peripheral_AFIO)   {}
  if (set & RCC_Peripheral_SYSCFG)   {
    apb2enr |= RCC_APB2ENR_SYSCFGCOMPEN;
  }
  if (set & RCC_Peripheral_GPIOA)  {
    ahbenr |= RCC_AHBENR_GPIOAEN;
  }
  if (set & RCC_Peripheral_GPIOB)  {
    ahbenr |= RCC_AHBENR_GPIOBEN;
  }
  if (set & RCC_Peripheral_GPIOC)  {
    ahbenr |= RCC_AHBENR_GPIOCEN;
  }
  if (set & RCC_Peripheral_GPIOD)  {
    ahbenr |= RCC_AHBENR_GPIODEN;
  }
  if (set & RCC_Peripheral_GPIOE)  {
    ahbenr |= RCC_AHBENR_GPIOEEN;
  }
  if (set & RCC_Peripheral_GPIOF)  {
    ahbenr |= RCC_AHBENR_GPIOFEN;
  }
  if (set & RCC_Peripheral_DAC) {
    apb1enr |= RCC_APB1ENR_DACEN;
  }
  if (set & RCC_Peripheral_USART1) {
    apb2enr |= RCC_APB2ENR_USART1EN;
  }
  if (set & RCC_Peripheral_USART2) {
    apb1enr |= RCC_APB1ENR_USART2EN;
  }
  if (set & RCC_Peripheral_SPI1)   {
    apb2enr |= RCC_APB2ENR_SPI1EN;
  }
  if (set & RCC_Peripheral_SPI2)   {
    apb1enr |= RCC_APB1ENR_SPI2EN;
  }
#elif defined(STM32F10X)
  if (set & RCC_Peripheral_ADC1)   {
    apb2enr |= RCC_APB2ENR_ADC1EN;
  }
  if (set & RCC_Peripheral_AFIO)   {
    apb2enr |= RCC_APB2ENR_AFIOEN;
  }
  if (set & RCC_Peripheral_SYSCFG)   {
  }
  if (set & RCC_Peripheral_GPIOA)  {
    apb2enr |= RCC_APB2ENR_IOPAEN;
  }
  if (set & RCC_Peripheral_GPIOB)  {
    apb2enr |= RCC_APB2ENR_IOPBEN;
  }
  if (set & RCC_Peripheral_GPIOC)  {
    apb2enr |= RCC_APB2ENR_IOPCEN;
  }
  if (set & RCC_Peripheral_GPIOD)  {
    apb2enr |= RCC_APB2ENR_IOPDEN;
  }
  if (set & RCC_Peripheral_GPIOE)  {
    apb2enr |= RCC_APB2ENR_IOPEEN;
  }
  if (set & RCC_Peripheral_GPIOF)  {
    assert_param(0);
  }
  if (set & RCC_Peripheral_DAC) {
#ifdef RCC_APB1ENR_DACEN
    apb1enr |= RCC_APB1ENR_DACEN;
#else
    assert_param(0);
#endif
  }
  if (set & RCC_Peripheral_USART1) {
    apb2enr |= RCC_APB2ENR_USART1EN;
  }
  if (set & RCC_Peripheral_USART2) {
    apb1enr |= RCC_APB1ENR_USART2EN;
  }
  if (set & RCC_Peripheral_SPI1)   {
    apb2enr |= RCC_APB2ENR_SPI1EN;
  }
  if (set & RCC_Peripheral_SPI2)   {
    apb1enr |= RCC_APB1ENR_SPI2EN;
  }
#else
#  error "No valid chip defined"
#endif

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

void RCC_getClocks(RCC_Clocks *clocks) {
#ifdef STM32F0XX
  uint32_t tmp = 0, pllmull = 0, pllsource = 0, prediv1factor = 0, presc = 0, pllclk = 0;

  /* Get SYSCLK source -------------------------------------------------------*/
  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch (tmp) {
  case 0x00:  /* HSI used as system clock */
    clocks->SYSCLK_Frequency = HSI_VALUE;
    break;
  case 0x04:  /* HSE used as system clock */
    clocks->SYSCLK_Frequency = HSE_VALUE;
    break;
  case 0x08:  /* PLL used as system clock */
    /* Get PLL clock source and multiplication factor ----------------------*/
    pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
    pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
    pllmull = (pllmull >> 18) + 2;

    if (pllsource == 0x00) {
      /* HSI oscillator clock divided by 2 selected as PLL clock entry */
      pllclk = (HSI_VALUE >> 1) * pllmull;
    } else {
      prediv1factor = (RCC->CFGR2 & RCC_CFGR2_PREDIV1) + 1;
      /* HSE oscillator clock selected as PREDIV1 clock entry */
      pllclk = (HSE_VALUE / prediv1factor) * pllmull;
    }
    clocks->SYSCLK_Frequency = pllclk;
    break;
  case 0x0C:  /* HSI48 used as system clock */
    clocks->SYSCLK_Frequency = HSI48_VALUE;
    break;
  default: /* HSI used as system clock */
    clocks->SYSCLK_Frequency = HSI_VALUE;
    break;
  }
  /* Compute HCLK, PCLK clocks frequencies -----------------------------------*/
  /* Get HCLK prescaler */
  tmp = RCC->CFGR & RCC_CFGR_HPRE;
  tmp = tmp >> 4;
  presc = APBAHBPrescTable[tmp];
  /* HCLK clock frequency */
  clocks->HCLK_Frequency = clocks->SYSCLK_Frequency >> presc;

  /* Get PCLK prescaler */
  tmp = RCC->CFGR & RCC_CFGR_PPRE;
  tmp = tmp >> 8;
  presc = APBAHBPrescTable[tmp];
  /* PCLK clock frequency */
  clocks->PCLK_Frequency = clocks->HCLK_Frequency >> presc;

  /* ADCCLK clock frequency */
  if ((RCC->CFGR3 & RCC_CFGR3_ADCSW) != RCC_CFGR3_ADCSW) {
    /* ADC Clock is HSI14 Osc. */
    clocks->ADCCLK_Frequency = HSI14_VALUE;
  } else {
    if ((RCC->CFGR & RCC_CFGR_ADCPRE) != RCC_CFGR_ADCPRE) {
      /* ADC Clock is derived from PCLK/2 */
      clocks->ADCCLK_Frequency = clocks->PCLK_Frequency >> 1;
    } else {
      /* ADC Clock is derived from PCLK/4 */
      clocks->ADCCLK_Frequency = clocks->PCLK_Frequency >> 2;
    }

  }

  /* CECCLK clock frequency */
  if ((RCC->CFGR3 & RCC_CFGR3_CECSW) != RCC_CFGR3_CECSW) {
    /* CEC Clock is HSI/244 */
    clocks->CECCLK_Frequency = HSI_VALUE / 244;
  } else {
    /* CECC Clock is LSE Osc. */
    clocks->CECCLK_Frequency = LSE_VALUE;
  }

  /* I2C1CLK clock frequency */
  if ((RCC->CFGR3 & RCC_CFGR3_I2C1SW) != RCC_CFGR3_I2C1SW) {
    /* I2C1 Clock is HSI Osc. */
    clocks->I2C1CLK_Frequency = HSI_VALUE;
  } else {
    /* I2C1 Clock is System Clock */
    clocks->I2C1CLK_Frequency = clocks->SYSCLK_Frequency;
  }

  /* USART1CLK clock frequency */
  if ((RCC->CFGR3 & RCC_CFGR3_USART1SW) == 0x0) {
    /* USART1 Clock is PCLK */
    clocks->USART1CLK_Frequency = clocks->PCLK_Frequency;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART1SW) == RCC_CFGR3_USART1SW_0) {
    /* USART1 Clock is System Clock */
    clocks->USART1CLK_Frequency = clocks->SYSCLK_Frequency;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART1SW) == RCC_CFGR3_USART1SW_1) {
    /* USART1 Clock is LSE Osc. */
    clocks->USART1CLK_Frequency = LSE_VALUE;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART1SW) == RCC_CFGR3_USART1SW) {
    /* USART1 Clock is HSI Osc. */
    clocks->USART1CLK_Frequency = HSI_VALUE;
  }

  /* USART2CLK clock frequency */
  if ((RCC->CFGR3 & RCC_CFGR3_USART2SW) == 0x0) {
    /* USART Clock is PCLK */
    clocks->USART2CLK_Frequency = clocks->PCLK_Frequency;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART2SW) == RCC_CFGR3_USART2SW_0) {
    /* USART Clock is System Clock */
    clocks->USART2CLK_Frequency = clocks->SYSCLK_Frequency;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART2SW) == RCC_CFGR3_USART2SW_1) {
    /* USART Clock is LSE Osc. */
    clocks->USART2CLK_Frequency = LSE_VALUE;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART2SW) == RCC_CFGR3_USART2SW) {
    /* USART Clock is HSI Osc. */
    clocks->USART2CLK_Frequency = HSI_VALUE;
  }

  /* USART3CLK clock frequency */
  if ((RCC->CFGR3 & RCC_CFGR3_USART3SW) == 0x0) {
    /* USART Clock is PCLK */
    clocks->USART3CLK_Frequency = clocks->PCLK_Frequency;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART3SW) == RCC_CFGR3_USART3SW_0) {
    /* USART Clock is System Clock */
    clocks->USART3CLK_Frequency = clocks->SYSCLK_Frequency;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART3SW) == RCC_CFGR3_USART3SW_1) {
    /* USART Clock is LSE Osc. */
    clocks->USART3CLK_Frequency = LSE_VALUE;
  } else if ((RCC->CFGR3 & RCC_CFGR3_USART3SW) == RCC_CFGR3_USART3SW) {
    /* USART Clock is HSI Osc. */
    clocks->USART3CLK_Frequency = HSI_VALUE;
  }

  /* USBCLK clock frequency */
  if ((RCC->CFGR3 & RCC_CFGR3_USBSW) != RCC_CFGR3_USBSW) {
    /* USB Clock is HSI48 */
    clocks->USBCLK_Frequency = HSI48_VALUE;
  } else {
    /* USB Clock is PLL clock */
    clocks->USBCLK_Frequency = pllclk;
  }
#elif defined (STM32F10X)
  uint32_t tmp = 0, pllmull = 0, pllsource = 0, presc = 0;

#ifdef  STM32F10X_CL
  uint32_t prediv1source = 0, prediv1factor = 0, prediv2factor = 0, pll2mull = 0;
#endif /* STM32F10X_CL */

#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
  uint32_t prediv1factor = 0;
#endif

  /* Get SYSCLK source -------------------------------------------------------*/
  tmp = RCC->CFGR & CFGR_SWS_Mask;

  switch (tmp) {
  case 0x00:  /* HSI used as system clock */
    clocks->SYSCLK_Frequency = HSI_VALUE;
    break;
  case 0x04:  /* HSE used as system clock */
    clocks->SYSCLK_Frequency = HSE_VALUE;
    break;
  case 0x08:  /* PLL used as system clock */

    /* Get PLL clock source and multiplication factor ----------------------*/
    pllmull = RCC->CFGR & CFGR_PLLMull_Mask;
    pllsource = RCC->CFGR & CFGR_PLLSRC_Mask;

#ifndef STM32F10X_CL
    pllmull = (pllmull >> 18) + 2;

    if (pllsource == 0x00) {
      /* HSI oscillator clock divided by 2 selected as PLL clock entry */
      clocks->SYSCLK_Frequency = (HSI_VALUE >> 1) * pllmull;
    } else {
#if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
      prediv1factor = (RCC->CFGR2 & CFGR2_PREDIV1) + 1;
      /* HSE oscillator clock selected as PREDIV1 clock entry */
      clocks->SYSCLK_Frequency = (HSE_VALUE / prediv1factor) * pllmull;
#else
      /* HSE selected as PLL clock entry */
      if ((RCC->CFGR & CFGR_PLLXTPRE_Mask) != (uint32_t)RESET) {
        /* HSE oscillator clock divided by 2 */
        clocks->SYSCLK_Frequency = (HSE_VALUE >> 1) * pllmull;
      } else {
        clocks->SYSCLK_Frequency = HSE_VALUE * pllmull;
      }
#endif
    }
#else
    pllmull = pllmull >> 18;

    if (pllmull != 0x0D) {
      pllmull += 2;
    } else {
      /* PLL multiplication factor = PLL input clock * 6.5 */
      pllmull = 13 / 2;
    }

    if (pllsource == 0x00) {
      /* HSI oscillator clock divided by 2 selected as PLL clock entry */
      clocks->SYSCLK_Frequency = (HSI_VALUE >> 1) * pllmull;
    } else {
      /* PREDIV1 selected as PLL clock entry */

      /* Get PREDIV1 clock source and division factor */
      prediv1source = RCC->CFGR2 & CFGR2_PREDIV1SRC;
      prediv1factor = (RCC->CFGR2 & CFGR2_PREDIV1) + 1;

      if (prediv1source == 0) {
        /* HSE oscillator clock selected as PREDIV1 clock entry */
        clocks->SYSCLK_Frequency = (HSE_VALUE / prediv1factor) * pllmull;
      } else {
        /* PLL2 clock selected as PREDIV1 clock entry */

        /* Get PREDIV2 division factor and PLL2 multiplication factor */
        prediv2factor = ((RCC->CFGR2 & CFGR2_PREDIV2) >> 4) + 1;
        pll2mull = ((RCC->CFGR2 & CFGR2_PLL2MUL) >> 8) + 2;
        clocks->SYSCLK_Frequency = (((HSE_VALUE / prediv2factor) * pll2mull) / prediv1factor) * pllmull;
      }
    }
#endif /* STM32F10X_CL */
    break;

  default:
    clocks->SYSCLK_Frequency = HSI_VALUE;
    break;
  }

  /* Compute HCLK, PCLK1, PCLK2 and ADCCLK clocks frequencies ----------------*/
  /* Get HCLK prescaler */
  tmp = RCC->CFGR & CFGR_HPRE_Set_Mask;
  tmp = tmp >> 4;
  presc = APBAHBPrescTable[tmp];
  /* HCLK clock frequency */
  clocks->HCLK_Frequency = clocks->SYSCLK_Frequency >> presc;
  /* Get PCLK1 prescaler */
  tmp = RCC->CFGR & CFGR_PPRE1_Set_Mask;
  tmp = tmp >> 8;
  presc = APBAHBPrescTable[tmp];
  /* PCLK1 clock frequency */
  clocks->PCLK1_Frequency = clocks->HCLK_Frequency >> presc;
  /* Get PCLK2 prescaler */
  tmp = RCC->CFGR & CFGR_PPRE2_Set_Mask;
  tmp = tmp >> 11;
  presc = APBAHBPrescTable[tmp];
  /* PCLK2 clock frequency */
  clocks->PCLK2_Frequency = clocks->HCLK_Frequency >> presc;
  /* Get ADCCLK prescaler */
  tmp = RCC->CFGR & CFGR_ADCPRE_Set_Mask;
  tmp = tmp >> 14;
  presc = ADCPrescTable[tmp];
  /* ADCCLK clock frequency */
  clocks->ADCCLK_Frequency = clocks->PCLK2_Frequency / presc;
#else
#  error "No valid chip defined"
#endif
}

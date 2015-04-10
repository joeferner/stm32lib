
#ifndef _STM32LIB_HAL_BASE_H_
#define _STM32LIB_HAL_BASE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports
  *         the name of the source file and the source line number of the call
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

#define assert_fail(str) { printf("%s\n", str); assert_failed((uint8_t *)__FILE__, __LINE__); }

/* Exported functions ------------------------------------------------------- */
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0)
#define assert_fail(str) { while(1); }
#endif /* USE_FULL_ASSERT */

#if defined (STM32F030x6) || defined (STM32F030x8) ||                           \
    defined (STM32F031x6) || defined (STM32F038xx) ||                           \
    defined (STM32F042x6) || defined (STM32F048xx) || defined (STM32F070x6) ||  \
    defined (STM32F051x8) || defined (STM32F058xx) ||                           \
    defined (STM32F071xB) || defined (STM32F072)   || defined (STM32F072xB) || defined (STM32F078xx) || defined (STM32F070xB) || \
    defined (STM32F091xC) || defined (STM32F098xx) || defined (STM32F030xC)
#  include "chip/stm32f0xx.h"
#  define STM32F0XX
#elif defined (STM32F103) || defined (STM32F10X_MD)
#  include "chip/stm32f10x.h"
#  define STM32F10X
#else
#  error "No valid chip defined"
#endif

#endif

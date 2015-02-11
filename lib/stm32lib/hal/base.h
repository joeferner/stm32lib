
#ifndef _STM32LIB_HAL_BASE_H_
#define _STM32LIB_HAL_BASE_H_

#include <stdint.h>
#include <stdbool.h>

#if defined (STM32F030x6) || defined (STM32F030x8) ||                           \
    defined (STM32F031x6) || defined (STM32F038xx) ||                           \
    defined (STM32F042x6) || defined (STM32F048xx) || defined (STM32F070x6) || \
    defined (STM32F051x8) || defined (STM32F058xx) ||                           \
    defined (STM32F071xB) || defined (STM32F072xB) || defined (STM32F078xx) || defined (STM32F070xB) || \
    defined (STM32F091xC) || defined (STM32F098xx) || defined (STM32F030xC)
#include "chip/stm32f0xx.h"
#else
#error "No valid chip defined"
#endif

#endif

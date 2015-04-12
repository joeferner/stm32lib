
#ifndef _STM32LIB_IWDG_H_
#define _STM32LIB_IWDG_H_

#ifdef IWDG_ENABLE
#  include "hal/iwdg.h"
#  define IWDG_RESET IWDG_reset();
void IWDG_setup(uint32_t ms);
#else
#  define IWDG_RESET
#endif

#endif

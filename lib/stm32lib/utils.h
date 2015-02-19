
#ifndef _STM32LIB_UTILS_H_
#define _STM32LIB_UTILS_H_

#ifndef clamp
#  define clamp(x, min, max) ( ((x) < (min)) ? (min) : ( ((x) > (max)) ? (max) : (x) ) )
#endif

#endif


#ifndef _STM32LIB_HAL_EXTI_H_
#define _STM32LIB_HAL_EXTI_H_

#include "base.h"

typedef enum {
  EXTI_Line_0  = 0x00000001,
  EXTI_Line_1  = 0x00000002,
  EXTI_Line_2  = 0x00000004,
  EXTI_Line_3  = 0x00000008,
  EXTI_Line_4  = 0x00000010,
  EXTI_Line_5  = 0x00000020,
  EXTI_Line_6  = 0x00000040,
  EXTI_Line_7  = 0x00000080,
  EXTI_Line_8  = 0x00000100,
  EXTI_Line_9  = 0x00000200,
  EXTI_Line_10 = 0x00000400,
  EXTI_Line_11 = 0x00000800,
  EXTI_Line_12 = 0x00001000,
  EXTI_Line_13 = 0x00002000,
  EXTI_Line_14 = 0x00004000,
  EXTI_Line_15 = 0x00008000,
  EXTI_Line_16 = 0x00010000,
  EXTI_Line_17 = 0x00020000,
  EXTI_Line_18 = 0x00040000,
  EXTI_Line_19 = 0x00080000,
  EXTI_Line_20 = 0x00100000,
  EXTI_Line_21 = 0x00200000,
  EXTI_Line_22 = 0x00400000,
  EXTI_Line_23 = 0x00800000,
  EXTI_Line_25 = 0x02000000,
  EXTI_Line_26 = 0x04000000,
  EXTI_Line_27 = 0x08000000,
  EXTI_Line_31 = 0x80000000
} EXTI_Line;
#define IS_EXTI_LINE(m) ( \
  ((m) == EXTI_Line_0) \
  || ((m) == EXTI_Line_1) \
  || ((m) == EXTI_Line_2) \
  || ((m) == EXTI_Line_3) \
  || ((m) == EXTI_Line_4) \
  || ((m) == EXTI_Line_5) \
  || ((m) == EXTI_Line_6) \
  || ((m) == EXTI_Line_7) \
  || ((m) == EXTI_Line_8) \
  || ((m) == EXTI_Line_9) \
  || ((m) == EXTI_Line_10) \
  || ((m) == EXTI_Line_11) \
  || ((m) == EXTI_Line_12) \
  || ((m) == EXTI_Line_13) \
  || ((m) == EXTI_Line_14) \
  || ((m) == EXTI_Line_15) \
  || ((m) == EXTI_Line_16) \
  || ((m) == EXTI_Line_17) \
  || ((m) == EXTI_Line_18) \
  || ((m) == EXTI_Line_19) \
  || ((m) == EXTI_Line_20) \
  || ((m) == EXTI_Line_21) \
  || ((m) == EXTI_Line_22) \
  || ((m) == EXTI_Line_23) \
  || ((m) == EXTI_Line_25) \
  || ((m) == EXTI_Line_26) \
  || ((m) == EXTI_Line_27) \
  || ((m) == EXTI_Line_31) \
)

typedef enum  {
  EXTI_Mode_interrupt,
  EXTI_Mode_event
} EXTI_Mode;
#define IS_EXTI_MODE(m) ( \
  ((m) == EXTI_Mode_interrupt) \
  || ((m) == EXTI_Mode_event) \
)

typedef enum {
  EXTI_Trigger_rising = 0x08,
  EXTI_Trigger_falling = 0x0C,
  EXTI_Trigger_both = 0x10
} EXTI_Trigger;
#define IS_EXTI_TRIGGER(m) ( \
  ((m) == EXTI_Trigger_rising) \
  || ((m) == EXTI_Trigger_falling) \
  || ((m) == EXTI_Trigger_both) \
)

typedef struct {
  EXTI_Line line;
  EXTI_Mode mode;
  EXTI_Trigger trigger;
} EXTI_InitParams;

void EXTI_initParamsInit(EXTI_InitParams *initParams);
void EXTI_enable(EXTI_InitParams *initParams);
FlagStatus EXTI_getStatus(EXTI_Line line);
void EXTI_clearPendingBit(EXTI_Line line);

#endif



#ifndef _STM32LIB_EXTI_H_
#define _STM32LIB_EXTI_H_

#include "hal/exti.h"
#include "hal/gpio.h"
#include "hal/base.h"

EXTI_Line EXTI_getLineForGpio(GPIO_Port port, GPIO_Pin pin);
IRQn_Type EXTI_getIRQForGpio(GPIO_Port port, GPIO_Pin pin);

#endif

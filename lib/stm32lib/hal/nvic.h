
#ifndef _STM32LIB_HAL_NVIC_H_
#define _STM32LIB_HAL_NVIC_H_

typedef enum {
  NVIC_PriorityGroup_0 = 0x700, /*!< 0 bits for pre-emption priority 4 bits for subpriority */
  NVIC_PriorityGroup_1 = 0x600, /*!< 1 bits for pre-emption priority 3 bits for subpriority */
  NVIC_PriorityGroup_2 = 0x500, /*!< 2 bits for pre-emption priority 2 bits for subpriority */
  NVIC_PriorityGroup_3 = 0x400, /*!< 3 bits for pre-emption priority 1 bits for subpriority */
  NVIC_PriorityGroup_4 = 0x300  /*!< 4 bits for pre-emption priority 0 bits for subpriority */
} NVIC_PriorityGroup;
#define IS_NVIC_PRIORITY_GROUP(v) ( \
    ((v) == NVIC_PriorityGroup_0) \
    || ((v) == NVIC_PriorityGroup_1) \
    || ((v) == NVIC_PriorityGroup_2) \
    || ((v) == NVIC_PriorityGroup_3) \
    || ((v) == NVIC_PriorityGroup_4) \
  )

void NVIC_priorityGroupConfig(NVIC_PriorityGroup priorityGroup);
void NVIC_enable(IRQn_Type irq, uint8_t preemptionPriority, uint8_t subPriority);
void NVIC_systemReset();

#endif


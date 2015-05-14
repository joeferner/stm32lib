
#include "base.h"
#include "nvic.h"

#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)
#define NVIC_AIRCR_VECTKEY    (0x5FA << 16)   /*!< AIRCR Key for write access   */
#define NVIC_SYSRESETREQ      2               /*!< System Reset Request         */

void NVIC_priorityGroupConfig(NVIC_PriorityGroup priorityGroup) {
  assert_param(IS_NVIC_PRIORITY_GROUP(priorityGroup));
  SCB->AIRCR = AIRCR_VECTKEY_MASK | priorityGroup;
}

void NVIC_enable(IRQn_Type irq, uint8_t preemptionPriority, uint8_t subPriority) {
  uint8_t tmppriority = 0x00, tmppre = 0x00, tmpsub = 0x0F;

  // Compute the Corresponding IRQ Priority
  tmppriority = (0x700 - ((SCB->AIRCR) & (uint32_t)0x700)) >> 0x08;
  tmppre = (0x4 - tmppriority);
  tmpsub = tmpsub >> tmppriority;

  tmppriority = (uint32_t)preemptionPriority << tmppre;
  tmppriority |= (uint8_t)(subPriority & tmpsub);
  tmppriority = tmppriority << 0x04;

  NVIC->IP[irq] = tmppriority;

  // Enable the Selected IRQ Channels
  NVIC->ISER[irq >> 0x05] = (uint32_t)0x01 << (irq & (uint8_t)0x1F);
}

void NVIC_systemReset() {
  SCB->AIRCR  = (NVIC_AIRCR_VECTKEY | (SCB->AIRCR & (0x700)) | (1 << NVIC_SYSRESETREQ));
  __DSB(); 
  while(1);  
}
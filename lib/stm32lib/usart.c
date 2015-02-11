
#include "usart.h"

void USART_txString(USART_Instance instance, const char *str) {
  const char *p = str;

  assert_param(str != NULL);

  while (*p) {
    USART_txWaitForComplete(instance);
    USART_tx(instance, *p);
    p++;
  }
}

void USART_txBytes(USART_Instance instance, uint8_t *data, uint32_t offset, uint32_t len) {
  uint32_t end = offset + len;
  uint32_t i = offset;

  assert_param(data != NULL);

  while (i < end) {
    USART_txWaitForComplete(instance);
    USART_tx(instance, data[i]);
    i++;
  }
}

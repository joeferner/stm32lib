/* Copyright (c) 2014, Nordic Semiconductor ASA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/** @file
@brief Implementation of the ACI transport layer module
*/

#include <inttypes.h>
#include "../../spi.h"
#include "../../hal/exti.h"
#include "../../time.h"
#include "hal_platform.h"
#include "hal_aci_tl.h"
#include "aci_queue.h"
/*
PIC32 supports only MSbit transfer on SPI and the nRF8001 uses LSBit
Use the REVERSE_BITS macro to convert from MSBit to LSBit
The outgoing command and the incoming event needs to be converted
*/
//Board dependent defines
#if defined (__AVR__)
    //For Arduino add nothing
#elif defined(__PIC32MX__)
    //For ChipKit as the transmission has to be reversed, the next definitions have to be added
    #define REVERSE_BITS(byte) (((reverse_lookup[(byte & 0x0F)]) << 4) + reverse_lookup[((byte & 0xF0) >> 4)])
    static const uint8_t reverse_lookup[] = { 0, 8,  4, 12, 2, 10, 6, 14,1, 9, 5, 13,3, 11, 7, 15 };
#endif

static void m_aci_data_print(hal_aci_data_t *p_data);
static void m_aci_event_check(void);
void m_aci_isr(void);
static void m_aci_pins_set(aci_pins_t *a_pins_ptr);
static inline void m_aci_reqn_disable (void);
static inline void m_aci_reqn_enable (void);
static void m_aci_q_flush(void);
static bool m_aci_spi_transfer(hal_aci_data_t * data_to_send, hal_aci_data_t * received_data);

static uint8_t        spi_readwrite(uint8_t aci_byte);

static bool           aci_debug_print = false;

EXTI_InitParams aci_rdyn_exti;
aci_queue_t    aci_tx_q;
aci_queue_t    aci_rx_q;

static aci_pins_t	 *a_pins_local_ptr;

void m_aci_data_print(hal_aci_data_t *p_data)
{
  const uint8_t length = p_data->buffer[0];
  uint8_t i;
  printf("%" PRIu8 " :", length);
  for (i=0; i<=length; i++)
  {
    printf("%02x, ", p_data->buffer[i]);
  }
  printf("\n");
}

/*
  Interrupt service routine called when the RDYN line goes low. Runs the SPI transfer.
*/
void m_aci_isr(void)
{
  hal_aci_data_t data_to_send;
  hal_aci_data_t received_data;

  // Receive from queue
  if (!aci_queue_dequeue_from_isr(&aci_tx_q, &data_to_send))
  {
    /* queue was empty, nothing to send */
    data_to_send.status_byte = 0;
    data_to_send.buffer[0] = 0;
  }

  // Receive and/or transmit data
  m_aci_spi_transfer(&data_to_send, &received_data);

  if (!aci_queue_is_full_from_isr(&aci_rx_q) && !aci_queue_is_empty_from_isr(&aci_tx_q))
  {
    m_aci_reqn_enable();
  }

  // Check if we received data
  if (received_data.buffer[0] > 0)
  {
    if (!aci_queue_enqueue_from_isr(&aci_rx_q, &received_data))
    {
      /* Receive Buffer full.
         Should never happen.
         Spin in a while loop.
      */
      while(1);
    }

    // Disable ready line interrupt until we have room to store incoming messages
    if (aci_queue_is_full_from_isr(&aci_rx_q))
    {
      EXTI_disable(&aci_rdyn_exti);
    }
  }

  return;
}

/*
  Checks the RDYN line and runs the SPI transfer if required.
*/
static void m_aci_event_check(void)
{
  hal_aci_data_t data_to_send;
  hal_aci_data_t received_data;

  // No room to store incoming messages
  if (aci_queue_is_full(&aci_rx_q))
  {
    return;
  }

  // If the ready line is disabled and we have pending messages outgoing we enable the request line
  if (GPIO_Bit_set == GPIO_readInputBit(a_pins_local_ptr->rdynPort, a_pins_local_ptr->rdynPin))
  {
    if (!aci_queue_is_empty(&aci_tx_q))
    {
      m_aci_reqn_enable();
    }

    return;
  }

  // Receive from queue
  if (!aci_queue_dequeue(&aci_tx_q, &data_to_send))
  {
    /* queue was empty, nothing to send */
    data_to_send.status_byte = 0;
    data_to_send.buffer[0] = 0;
  }

  // Receive and/or transmit data
  m_aci_spi_transfer(&data_to_send, &received_data);

  /* If there are messages to transmit, and we can store the reply, we request a new transfer */
  if (!aci_queue_is_full(&aci_rx_q) && !aci_queue_is_empty(&aci_tx_q))
  {
    m_aci_reqn_enable();
  }

  // Check if we received data
  if (received_data.buffer[0] > 0)
  {
    if (!aci_queue_enqueue(&aci_rx_q, &received_data))
    {
      /* Receive Buffer full.
         Should never happen.
         Spin in a while loop.
      */
      while(1);
    }
  }

  return;
}

/** @brief Point the low level library at the ACI pins specified
 *  @details
 *  The ACI pins are specified in the application and a pointer is made available for
 *  the low level library to use
 */
static void m_aci_pins_set(aci_pins_t *a_pins_ptr)
{
  a_pins_local_ptr = a_pins_ptr;
}

static inline void m_aci_reqn_disable (void)
{
  GPIO_setBits(a_pins_local_ptr->csPort, a_pins_local_ptr->csPin);
}

static inline void m_aci_reqn_enable (void)
{
  GPIO_resetBits(a_pins_local_ptr->csPort, a_pins_local_ptr->csPin);
}

static void m_aci_q_flush(void)
{
  __disable_irq();
  /* re-initialize aci cmd queue and aci event queue to flush them*/
  aci_queue_init(&aci_tx_q);
  aci_queue_init(&aci_rx_q);
  __enable_irq();
}

static bool m_aci_spi_transfer(hal_aci_data_t * data_to_send, hal_aci_data_t * received_data)
{
  uint8_t byte_cnt;
  uint8_t byte_sent_cnt;
  uint8_t max_bytes;

  m_aci_reqn_enable();

  // Send length, receive header
  byte_sent_cnt = 0;
  received_data->status_byte = spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  // Send first byte, receive length from slave
  received_data->buffer[0] = spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  if (0 == data_to_send->buffer[0])
  {
    max_bytes = received_data->buffer[0];
  }
  else
  {
    // Set the maximum to the biggest size. One command byte is already sent
    max_bytes = (received_data->buffer[0] > (data_to_send->buffer[0] - 1))
                                          ? received_data->buffer[0]
                                          : (data_to_send->buffer[0] - 1);
  }

  if (max_bytes > HAL_ACI_MAX_LENGTH)
  {
    max_bytes = HAL_ACI_MAX_LENGTH;
  }

  // Transmit/receive the rest of the packet
  for (byte_cnt = 0; byte_cnt < max_bytes; byte_cnt++)
  {
    received_data->buffer[byte_cnt+1] =  spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  }

  // RDYN should follow the REQN line in approx 100ns
  m_aci_reqn_disable();

  return (max_bytes > 0);
}

void hal_aci_tl_debug_print(bool enable)
{
	aci_debug_print = enable;
}

void hal_aci_tl_pin_reset(void)
{
  GPIO_InitParams initParams;
  
  GPIO_initParamsInit(&initParams);
  initParams.port = a_pins_local_ptr->resetPort;
  initParams.pin = a_pins_local_ptr->resetPin;
  initParams.mode = GPIO_Mode_output;
  GPIO_init(&initParams);

  GPIO_setBits(a_pins_local_ptr->resetPort, a_pins_local_ptr->resetPin);
  sleep_ms(10);
  GPIO_resetBits(a_pins_local_ptr->resetPort, a_pins_local_ptr->resetPin);
  sleep_ms(10);
  GPIO_setBits(a_pins_local_ptr->resetPort, a_pins_local_ptr->resetPin);
}

bool hal_aci_tl_event_peek(hal_aci_data_t *p_aci_data)
{
  if (!a_pins_local_ptr->interface_is_interrupt)
  {
    m_aci_event_check();
  }

  if (aci_queue_peek(&aci_rx_q, p_aci_data))
  {
    return true;
  }

  return false;
}

bool hal_aci_tl_event_get(hal_aci_data_t *p_aci_data)
{
  bool was_full;

  if (!a_pins_local_ptr->interface_is_interrupt && !aci_queue_is_full(&aci_rx_q))
  {
    m_aci_event_check();
  }

  was_full = aci_queue_is_full(&aci_rx_q);

  if (aci_queue_dequeue(&aci_rx_q, p_aci_data))
  {
    if (aci_debug_print)
    {
      printf(" E");
      m_aci_data_print(p_aci_data);
    }

    if (was_full && a_pins_local_ptr->interface_is_interrupt)
	  {
      /* Enable RDY line interrupt again */
      EXTI_enable(&aci_rdyn_exti);
    }

    /* Attempt to pull REQN LOW since we've made room for new messages */
    if (!aci_queue_is_full(&aci_rx_q) && !aci_queue_is_empty(&aci_tx_q))
    {
      m_aci_reqn_enable();
    }

    return true;
  }

  return false;
}

void hal_aci_tl_init(aci_pins_t *a_pins, bool debug)
{
  aci_debug_print = debug;

  /* Needs to be called as the first thing for proper intialization*/
  m_aci_pins_set(a_pins);

  /*
  The SPI lines used are mapped directly to the hardware SPI
  MISO MOSI and SCK
  Change here if the pins are mapped differently

  The SPI library assumes that the hardware pins are used
  */
  SPI_InitParams spi;
  SPI_initParamsInit(&spi);
  spi.sckPort = a_pins->sckPort;
  spi.sckPin = a_pins->sckPin;
  spi.mosiPort = a_pins->mosiPort;
  spi.mosiPin = a_pins->mosiPin;
  spi.misoPort = a_pins->misoPort;
  spi.misoPin = a_pins->misoPin;
  spi.halSpiInitParams.instance = a_pins->spi;
  spi.halSpiInitParams.direction = SPI_Direction_2LinesFullDuplex;
  spi.halSpiInitParams.mode = SPI_Mode_master;
  spi.halSpiInitParams.dataSize = SPI_DataSize_8b;
  spi.halSpiInitParams.baudRatePrescaler = a_pins->spi_clock_divider;
  spi.halSpiInitParams.firstBit = SPI_FirstBit_lsb;
  spi.halSpiInitParams.cpol = SPI_CPOL_low;
  spi.halSpiInitParams.cpha = SPI_CPHA_1Edge;
  SPI_init(&spi);

  /* Initialize the ACI Command queue. This must be called after the delay above. */
  aci_queue_init(&aci_tx_q);
  aci_queue_init(&aci_rx_q);

  //Configure the IO lines
  GPIO_InitParams gpio;
  GPIO_initParamsInit(&gpio);
  gpio.port = a_pins->rdynPort;
  gpio.pin = a_pins->rdynPin;
  gpio.mode = GPIO_Mode_input;
  gpio.pullUpDown = GPIO_PullUpDown_pullUp;
  GPIO_init(&gpio);

  GPIO_initParamsInit(&gpio);
  gpio.port = a_pins->csPort;
  gpio.pin = a_pins->csPin;
  gpio.mode = GPIO_Mode_output;
  GPIO_init(&gpio);

  GPIO_initParamsInit(&gpio);
  gpio.port = a_pins->activePort;
  gpio.pin = a_pins->activePin;
  gpio.mode = GPIO_Mode_input;
  GPIO_init(&gpio);

  /* Pin reset the nRF8001, required when the nRF8001 setup is being changed */
  hal_aci_tl_pin_reset();

  /* Set the nRF8001 to a known state as required by the datasheet*/
  GPIO_resetBits(a_pins->misoPort, a_pins->misoPin);
  GPIO_resetBits(a_pins->mosiPort, a_pins->mosiPin);
  GPIO_setBits(a_pins->csPort, a_pins->csPin);
  GPIO_resetBits(a_pins->sckPort, a_pins->sckPin);

  EXTI_initParamsInit(&aci_rdyn_exti);
  aci_rdyn_exti.line    = a_pins->rdynExti;
  aci_rdyn_exti.trigger = EXTI_Trigger_falling;
  
  sleep_ms(30); //Wait for the nRF8001 to get hold of its lines - the lines float for a few ms after the reset

  /* Attach the interrupt to the RDYN line as requested by the caller */
  if (a_pins->interface_is_interrupt)
  {
    // We use the LOW level of the RDYN line as the atmega328 can wakeup from sleep only on LOW
    EXTI_enable(&aci_rdyn_exti);
  }
}

bool hal_aci_tl_send(hal_aci_data_t *p_aci_cmd)
{
  const uint8_t length = p_aci_cmd->buffer[0];
  bool ret_val = false;

  if (length > HAL_ACI_MAX_LENGTH)
  {
    return false;
  }

  ret_val = aci_queue_enqueue(&aci_tx_q, p_aci_cmd);
  if (ret_val)
  {
    if(!aci_queue_is_full(&aci_rx_q))
    {
      // Lower the REQN only when successfully enqueued
      m_aci_reqn_enable();
    }

    if (aci_debug_print)
    {
      printf("C"); //ACI Command
      m_aci_data_print(p_aci_cmd);
    }
  }

  return ret_val;
}

static uint8_t spi_readwrite(const uint8_t aci_byte)
{
  return SPI_transfer(a_pins_local_ptr->spi, aci_byte);
}

bool hal_aci_tl_rx_q_empty (void)
{
  return aci_queue_is_empty(&aci_rx_q);
}

bool hal_aci_tl_rx_q_full (void)
{
  return aci_queue_is_full(&aci_rx_q);
}

bool hal_aci_tl_tx_q_empty (void)
{
  return aci_queue_is_empty(&aci_tx_q);
}

bool hal_aci_tl_tx_q_full (void)
{
  return aci_queue_is_full(&aci_tx_q);
}

void hal_aci_tl_q_flush (void)
{
  m_aci_q_flush();
}


#include "platform_config.h"
#include <stdio.h>
#include <stm32lib/device/nRF8001/lib_aci.h>
#include <stm32lib/device/nRF8001/aci_setup.h>
#include <stm32lib/device/nRF8001/uart_over_ble.h>
#include "bluetooth-services.h"

static services_pipe_type_mapping_t services_pipe_type_mapping[NUMBER_OF_PIPES] = SERVICES_PIPE_TYPE_MAPPING_CONTENT;
static struct aci_state_t aci_state;
static const hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] = SETUP_MESSAGES_CONTENT;
static hal_aci_evt_t aci_data;
static bool timing_change_done = false;
static uart_over_ble_t uart_over_ble;
static uint8_t uart_buffer[20];
static uint8_t uart_buffer_len = 0;

static void setup();
static void loop();
bool uart_process_control_point_rx(uint8_t *byte, uint8_t length);
void bluetooth_setup();
void bluetooth_tick();

int main(void) {
  setup();
  while (1) {
    loop();
  }
  return 0;
}

static void setup() {
  debug_setup();
  bluetooth_setup();
  printf("setup complete!\n");
}

static void loop() {
  bluetooth_tick();
}

void bluetooth_setup() {
  bool debug = true;

  printf("BEGIN bluetooth setup\n");
  
  aci_state.aci_setup_info.services_pipe_type_mapping = &services_pipe_type_mapping[0];
  aci_state.aci_setup_info.number_of_pipes    = NUMBER_OF_PIPES;
  aci_state.aci_setup_info.setup_msgs         = (hal_aci_data_t*)setup_msgs;
  aci_state.aci_setup_info.num_setup_msgs     = NB_SETUP_MESSAGES;
  
  aci_state.aci_pins.spi        = BLUETOOTH_SPI;
  aci_state.aci_pins.rdynPort   = BLUETOOTH_RDYN_PORT;
  aci_state.aci_pins.rdynPin    = BLUETOOTH_RDYN_PIN;
  aci_state.aci_pins.rdynExti   = BLUETOOTH_RDYN_EXTI;
  aci_state.aci_pins.mosiPort   = BLUETOOTH_MOSI_PORT;
  aci_state.aci_pins.mosiPin    = BLUETOOTH_MOSI_PIN;
  aci_state.aci_pins.misoPort   = BLUETOOTH_MISO_PORT;
  aci_state.aci_pins.misoPin    = BLUETOOTH_MISO_PIN;
  aci_state.aci_pins.sckPort    = BLUETOOTH_SCK_PORT;
  aci_state.aci_pins.sckPin     = BLUETOOTH_SCK_PIN;
  aci_state.aci_pins.csPort     = BLUETOOTH_CS_PORT;
  aci_state.aci_pins.csPin      = BLUETOOTH_CS_PIN;
  aci_state.aci_pins.resetPort  = BLUETOOTH_RESET_PORT;
  aci_state.aci_pins.resetPin   = BLUETOOTH_RESET_PIN;
  aci_state.aci_pins.activePort = BLUETOOTH_ACTIVE_PORT;
  aci_state.aci_pins.activePin  = BLUETOOTH_ACTIVE_PIN;
  aci_state.aci_pins.spi_clock_divider      = BLUETOOTH_SPI_PRESCALER;
  aci_state.aci_pins.interface_is_interrupt = false;

  lib_aci_init(&aci_state, debug);
  
  printf("END bluetooth setup\n");
}

void bluetooth_tick() {
  static bool setup_required = false;

  // We enter the if statement only when there is a ACI event available to be processed
  if (lib_aci_event_get(&aci_state, &aci_data)) {
    aci_evt_t *aci_evt;
    aci_evt = &aci_data.evt;

    switch(aci_evt->evt_opcode) {
      /**
      As soon as you reset the nRF8001 you will get an ACI Device Started Event
      */
      case ACI_EVT_DEVICE_STARTED:
        aci_state.data_credit_total = aci_evt->params.device_started.credit_available;
        switch(aci_evt->params.device_started.device_mode) {
          case ACI_DEVICE_SETUP:
            /**
            When the device is in the setup mode
            */
            printf("Evt Device Started: Setup\n");
            setup_required = true;
            break;

          case ACI_DEVICE_STANDBY:
            printf("Evt Device Started: Standby\n");
            //Looking for an iPhone by sending radio advertisements
            //When an iPhone connects to us we will get an ACI_EVT_CONNECTED event from the nRF8001
            if (aci_evt->params.device_started.hw_error) {
              sleep_us(20); //Magic number used to make sure the HW error event is handled correctly.
            } else {
	      lib_aci_connect(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
	      printf("Advertising started\n");
            }
            break;
        }
        break; //ACI Device Started Event

      case ACI_EVT_CMD_RSP:
        //If an ACI command response event comes with an error -> stop
        if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status) {
          //ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
          //TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
          //all other ACI commands will have status code of ACI_STATUS_SUCCESS for a successful command
          printf("ACI Command 0x%02X\n", aci_evt->params.cmd_rsp.cmd_opcode);
          printf("Evt Cmd respone: Status 0x%02X\n", aci_evt->params.cmd_rsp.cmd_status);
        }
        if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode) {
          //Store the version and configuration information of the nRF8001 in the Hardware Revision String Characteristic
          lib_aci_set_local_data(&aci_state, PIPE_DEVICE_INFORMATION_HARDWARE_REVISION_STRING_SET,
            (uint8_t *)&(aci_evt->params.cmd_rsp.params.get_device_version), sizeof(aci_evt_cmd_rsp_params_get_device_version_t));
        }
        break;

      case ACI_EVT_CONNECTED:
        printf("Evt Connected\n");
        uart_over_ble.uart_rts_local = true;
        timing_change_done = false;
        aci_state.data_credit_available = aci_state.data_credit_total;

        /*
        Get the device version of the nRF8001 and store it in the Hardware Revision String
        */
        lib_aci_device_version();
        break;

      case ACI_EVT_PIPE_STATUS:
        printf("Evt Pipe Status\n");
        if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX) && (false == timing_change_done)) {
          lib_aci_change_timing_GAP_PPCP(); // change the timing on the link as specified in the nRFgo studio -> nRF8001 conf. -> GAP.
                                            // Used to increase or decrease bandwidth
          timing_change_done = true;
        }
        break;

      case ACI_EVT_TIMING:
        printf("Evt link connection interval changed\n");
        lib_aci_set_local_data(&aci_state,
                                PIPE_UART_OVER_BTLE_UART_LINK_TIMING_CURRENT_SET,
                                (uint8_t *)&(aci_evt->params.timing.conn_rf_interval), /* Byte aligned */
                                PIPE_UART_OVER_BTLE_UART_LINK_TIMING_CURRENT_SET_MAX_SIZE);
        break;

      case ACI_EVT_DISCONNECTED:
        printf("Evt Disconnected/Advertising timed out\n");
        lib_aci_connect(180/* in seconds */, 0x0100 /* advertising interval 100ms*/);
        printf("Advertising started");
        break;

      case ACI_EVT_DATA_RECEIVED:
        printf("Pipe Number: %d\n", aci_evt->params.data_received.rx_data.pipe_number);
        if (PIPE_UART_OVER_BTLE_UART_RX_RX == aci_evt->params.data_received.rx_data.pipe_number) {
          printf(" Data(Hex) : ");
          for(int i=0; i<aci_evt->len - 2; i++) {
            printf("%c ", (char)aci_evt->params.data_received.rx_data.aci_data[i]);
            uart_buffer[i] = aci_evt->params.data_received.rx_data.aci_data[i];
          }

          uart_buffer_len = aci_evt->len - 2;
          printf("\n");
          if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX)) {
            /*Do this to test the loopback otherwise comment it out
            */
            /*
            if (!uart_tx(&uart_buffer[0], aci_evt->len - 2)) {
              printf("UART loopback failed\n"));
            } else {
              printf("UART loopback OK\n");
            }
            */
          }
        }
        if (PIPE_UART_OVER_BTLE_UART_CONTROL_POINT_RX == aci_evt->params.data_received.rx_data.pipe_number) {
            uart_process_control_point_rx(&aci_evt->params.data_received.rx_data.aci_data[0], aci_evt->len - 2); //Subtract for Opcode and Pipe number
        }
        break;

      case ACI_EVT_DATA_CREDIT:
        aci_state.data_credit_available = aci_state.data_credit_available + aci_evt->params.data_credit.credit;
        break;

      case ACI_EVT_PIPE_ERROR:
        //See the appendix in the nRF8001 Product Specication for details on the error codes
        printf("ACI Evt Pipe Error: Pipe #: %d\n", aci_evt->params.pipe_error.pipe_number);
        printf("  Pipe Error Code: 0x%02X\n", aci_evt->params.pipe_error.error_code);

        //Increment the credit available as the data packet was not sent.
        //The pipe error also represents the Attribute protocol Error Response sent from the peer and that should not be counted
        //for the credit.
        if (ACI_STATUS_ERROR_PEER_ATT_ERROR != aci_evt->params.pipe_error.error_code) {
          aci_state.data_credit_available++;
        }
        break;

      case ACI_EVT_HW_ERROR:
        printf("HW error: %d\n", aci_evt->params.hw_error.line_num);

        for(uint8_t counter = 0; counter <= (aci_evt->len - 3); counter++) {
          printf("%c", aci_evt->params.hw_error.file_name[counter]); //uint8_t file_name[20];
        }
        printf("\n");
        lib_aci_connect(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
        printf("Advertising started\n");
        break;
    }
  } else {
    //printf("No ACI Events available\n");
    // No event in the ACI Event queue and if there is no event in the ACI command queue the arduino can go to sleep
    // Arduino can go to sleep now
    // Wakeup from sleep from the RDYN line
  }

  /* setup_required is set to true when the device starts up and enters setup mode.
   * It indicates that do_aci_setup() should be called. The flag should be cleared if
   * do_aci_setup() returns ACI_STATUS_TRANSACTION_COMPLETE.
   */
  if(setup_required) {
    if (SETUP_SUCCESS == do_aci_setup(&aci_state)) {
      setup_required = false;
    }
  }
}

bool uart_process_control_point_rx(uint8_t *byte, uint8_t length) {
  bool status = false;
  aci_ll_conn_params_t *conn_params;

  if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_CONTROL_POINT_TX) ) {
    printf("%02X", *byte);
    switch(*byte) {
      /*
      Queues a ACI Disconnect to the nRF8001 when this packet is received.
      May cause some of the UART packets being sent to be dropped
      */
      case UART_OVER_BLE_DISCONNECT:
        /*
        Parameters:
        None
        */
        lib_aci_disconnect(&aci_state, ACI_REASON_TERMINATE);
        status = true;
        break;

      /*
      Queues an ACI Change Timing to the nRF8001
      */
      case UART_OVER_BLE_LINK_TIMING_REQ:
        /*
        Parameters:
        Connection interval min: 2 bytes
        Connection interval max: 2 bytes
        Slave latency:           2 bytes
        Timeout:                 2 bytes
        Same format as Peripheral Preferred Connection Parameters (See nRFgo studio -> nRF8001 Configuration -> GAP Settings
        Refer to the ACI Change Timing Request in the nRF8001 Product Specifications
        */
        conn_params = (aci_ll_conn_params_t *)(byte+1);
        lib_aci_change_timing(
	  conn_params->min_conn_interval,
	  conn_params->max_conn_interval,
	  conn_params->slave_latency,
	  conn_params->timeout_mult
	);
        status = true;
        break;

      /*
      Clears the RTS of the UART over BLE
      */
      case UART_OVER_BLE_TRANSMIT_STOP:
        /*
        Parameters:
        None
        */
        uart_over_ble.uart_rts_local = false;
        status = true;
        break;

      /*
      Set the RTS of the UART over BLE
      */
      case UART_OVER_BLE_TRANSMIT_OK:
        /*
        Parameters:
        None
        */
        uart_over_ble.uart_rts_local = true;
        status = true;
        break;
    }
  }
  return status;
}

void __ble_assert(const char *file, uint16_t line) {
  printf("Bluetooth ERROR: %s:%d\n", file, line);
  while(1);
}

void assert_failed(uint8_t *file, uint32_t line) {
  while (1) {
    printf("assert_failed: %s:%lu\n", file, line);
  }
}

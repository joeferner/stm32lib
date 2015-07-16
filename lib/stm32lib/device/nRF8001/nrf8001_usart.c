
#include "nrf8001_usart.h"
#include "../../time.h"
#include <stdio.h>
#include "aci_setup.h"
#include "uart_over_ble.h"
#include "bluetooth-services-usart.h"

#ifdef NRF8001_DEBUG
#  define nrf8001_printf(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
#  define nrf8001_printf(fmt, ...)  
#endif

static hal_aci_evt_t aci_data;
static services_pipe_type_mapping_t services_pipe_type_mapping[NUMBER_OF_PIPES] = SERVICES_PIPE_TYPE_MAPPING_CONTENT;
static const hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] = SETUP_MESSAGES_CONTENT;
static bool timing_change_done = false;
static uart_over_ble_t uart_over_ble;

bool uart_process_control_point_rx(uint8_t *byte, uint8_t length);

void nrf8001_usart_setup(bool debug) {
  aci_state.aci_setup_info.services_pipe_type_mapping = &services_pipe_type_mapping[0];
  aci_state.aci_setup_info.number_of_pipes    = NUMBER_OF_PIPES;
  aci_state.aci_setup_info.setup_msgs         = (hal_aci_data_t*)setup_msgs;
  aci_state.aci_setup_info.num_setup_msgs     = NB_SETUP_MESSAGES;

  lib_aci_init(&aci_state, debug);
}

bool nrf8001_usart_tx(const uint8_t* buffer, uint16_t len) {
  return lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)buffer, len);
}

void nrf8001_usart_tick() {
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
            nrf8001_printf("Evt Device Started: Setup\n");
            setup_required = true;
            break;

          case ACI_DEVICE_STANDBY:
            nrf8001_printf("Evt Device Started: Standby\n");
            //Looking for an iPhone by sending radio advertisements
            //When an iPhone connects to us we will get an ACI_EVT_CONNECTED event from the nRF8001
            if (aci_evt->params.device_started.hw_error) {
              sleep_us(20); //Magic number used to make sure the HW error event is handled correctly.
            } else {
	      lib_aci_connect(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
	      nrf8001_printf("Advertising started\n");
            }
            break;
	    
          case ACI_DEVICE_INVALID:
	  case ACI_DEVICE_TEST:
	  case ACI_DEVICE_SLEEP:
	    break;
        }
        break; //ACI Device Started Event

      case ACI_EVT_CMD_RSP:
        //If an ACI command response event comes with an error -> stop
        if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status) {
          //ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
          //TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
          //all other ACI commands will have status code of ACI_STATUS_SUCCESS for a successful command
          nrf8001_printf("ACI Command 0x%02X\n", aci_evt->params.cmd_rsp.cmd_opcode);
          nrf8001_printf("Evt Cmd respone: Status 0x%02X\n", aci_evt->params.cmd_rsp.cmd_status);
        }
        if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode) {
          //Store the version and configuration information of the nRF8001 in the Hardware Revision String Characteristic
          lib_aci_set_local_data(&aci_state, PIPE_DEVICE_INFORMATION_HARDWARE_REVISION_STRING_SET,
            (uint8_t *)&(aci_evt->params.cmd_rsp.params.get_device_version), sizeof(aci_evt_cmd_rsp_params_get_device_version_t));
        }
        break;

      case ACI_EVT_CONNECTED:
        nrf8001_printf("Evt Connected\n");
        uart_over_ble.uart_rts_local = true;
        timing_change_done = false;
        aci_state.data_credit_available = aci_state.data_credit_total;

        /*
        Get the device version of the nRF8001 and store it in the Hardware Revision String
        */
        lib_aci_device_version();
        break;

      case ACI_EVT_PIPE_STATUS:
        nrf8001_printf("Evt Pipe Status\n");
        if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX) && (false == timing_change_done)) {
          lib_aci_change_timing_GAP_PPCP(); // change the timing on the link as specified in the nRFgo studio -> nRF8001 conf. -> GAP.
                                            // Used to increase or decrease bandwidth
          timing_change_done = true;
        }
        break;

      case ACI_EVT_TIMING:
        nrf8001_printf("Evt link connection interval changed\n");
        lib_aci_set_local_data(&aci_state,
                                PIPE_UART_OVER_BTLE_UART_LINK_TIMING_CURRENT_SET,
                                (uint8_t *)&(aci_evt->params.timing.conn_rf_interval), /* Byte aligned */
                                PIPE_UART_OVER_BTLE_UART_LINK_TIMING_CURRENT_SET_MAX_SIZE);
        break;

      case ACI_EVT_DISCONNECTED:
        nrf8001_printf("Evt Disconnected/Advertising timed out\n");
        lib_aci_connect(180/* in seconds */, 0x0100 /* advertising interval 100ms*/);
        nrf8001_printf("Advertising started");
        break;

      case ACI_EVT_DATA_RECEIVED:
        nrf8001_printf("Pipe Number: %d\n", aci_evt->params.data_received.rx_data.pipe_number);
        if (PIPE_UART_OVER_BTLE_UART_RX_RX == aci_evt->params.data_received.rx_data.pipe_number) {
          nrf8001_printf(" Data(Hex) : ");
          for(int i=0; i<aci_evt->len - 2; i++) {
            nrf8001_printf("%c ", (char)aci_evt->params.data_received.rx_data.aci_data[i]);
	    RingBufferU8_writeByte(&nrf8001_usart_rxBuffer, aci_evt->params.data_received.rx_data.aci_data[i]);
          }
          nrf8001_printf("\n");
	  nrf8001_usart_rx();
          if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX)) {
            /*Do this to test the loopback otherwise comment it out
            */
            /*
            if (!uart_tx(&uart_buffer[0], aci_evt->len - 2)) {
              nrf8001_printf("UART loopback failed\n"));
            } else {
              nrf8001_printf("UART loopback OK\n");
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
        nrf8001_printf("ACI Evt Pipe Error: Pipe #: %d\n", aci_evt->params.pipe_error.pipe_number);
        nrf8001_printf("  Pipe Error Code: 0x%02X\n", aci_evt->params.pipe_error.error_code);

        //Increment the credit available as the data packet was not sent.
        //The pipe error also represents the Attribute protocol Error Response sent from the peer and that should not be counted
        //for the credit.
        if (ACI_STATUS_ERROR_PEER_ATT_ERROR != aci_evt->params.pipe_error.error_code) {
          aci_state.data_credit_available++;
        }
        break;

      case ACI_EVT_HW_ERROR:
        nrf8001_printf("HW error: %d\n", aci_evt->params.hw_error.line_num);

        for(uint8_t counter = 0; counter <= (aci_evt->len - 3); counter++) {
          nrf8001_printf("%c", aci_evt->params.hw_error.file_name[counter]); //uint8_t file_name[20];
        }
        nrf8001_printf("\n");
        lib_aci_connect(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
        nrf8001_printf("Advertising started\n");
        break;
	
      case ACI_EVT_INVALID:
      case ACI_EVT_ECHO:
      case ACI_EVT_BOND_STATUS:
      case ACI_EVT_DATA_ACK:
      case ACI_EVT_DISPLAY_PASSKEY:
      case ACI_EVT_KEY_REQUEST:
	break;
    }
  } else {
    //nrf8001_printf("No ACI Events available\n");
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
    nrf8001_printf("%02X", *byte);
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
  nrf8001_printf("Bluetooth ERROR: %s:%d\n", file, line);
  while(1);
}

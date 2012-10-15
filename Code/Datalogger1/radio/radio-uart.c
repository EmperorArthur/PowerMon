#include <avr/io.h>
#include "errcode.h"
#include "led.h"
#include <avr/eeprom.h>
#include "radio-uart.h"

#define EEPROM_CHAN ((uint8_t *)0x00)
#define EEPROM_PAN ((uint16_t *)0x02)
#define EEPROM_ADDR ((uint16_t *)0x04)

//I'm using an AT86RF231, so I have some extra features, like some LED Outs
#define PA_EXT_EN				0b10000000
#define TRX_CTRL_1				(0x04)

static uint8_t radio_buffer[129];
static uint8_t radio_position;
hal_rx_frame_t frame;

typedef enum{
    
    TIME_TO_ENTER_P_ON               = 510, //!< Transition time from VCC is applied to P_ON.
    TIME_P_ON_TO_TRX_OFF             = 510, //!< Transition time from P_ON to TRX_OFF.
    TIME_SLEEP_TO_TRX_OFF            = 880, //!< Transition time from SLEEP to TRX_OFF.
    TIME_RESET                       = 6,   //!< Time to hold the RST pin low during reset
    TIME_ED_MEASUREMENT              = 140, //!< Time it takes to do a ED measurement.
    TIME_CCA                         = 140, //!< Time it takes to do a CCA.
    TIME_PLL_LOCK                    = 150, //!< Maximum time it should take for the PLL to lock.
    TIME_FTN_TUNING                  = 25,  //!< Maximum time it should take to do the filter tuning.
    TIME_NOCLK_TO_WAKE               = 6,   //!< Transition time from *_NOCLK to being awake.
    TIME_CMD_FORCE_TRX_OFF           = 1,    //!< Time it takes to execute the FORCE_TRX_OFF command.
    TIME_TRX_OFF_TO_PLL_ACTIVE       = 180, //!< Transition time from TRX_OFF to: RX_ON, PLL_ON, TX_ARET_ON and RX_AACK_ON.
    TIME_STATE_TRANSITION_PLL_ACTIVE = 1, //!< Transition time from PLL active state to another.
}tat_trx_timing_t;

//This is an extremely simplified radio_send_frame(...)
uint8_t send(uint8_t buf[],uint8_t length){	
	length=length+2;	//To account for the CRC the radio appends
	
	if(local_tat_set_trx_state(TX_ARET_ON) == TAT_SUCCESS) {
		radio_led_on();
		send_data(length, buf);
		radio_led_off();
		return 0;
	} else {
		return 1;
	}
}

//This puts a charachter in the buffer
//When the buffer is full it sends the data
void radio_putchar_f(char c, FILE *stream){
	radio_buffer[radio_position++] = c;
	if(radio_position > 120){
		send(radio_buffer,radio_position-1);
		send(radio_buffer,radio_position-1);
		local_radio_init();
		radio_position = 0;
	}
}

void radio_putchar(char c){
	radio_buffer[radio_position++] = c;
	if(radio_position > 120){
		send(radio_buffer,radio_position-1);
		send(radio_buffer,radio_position-1);
		local_radio_init();
		radio_position = 0;
	}
}

void radio_transmit(){
	if(radio_position > 0){
		send(radio_buffer,radio_position-1);
		send(radio_buffer,radio_position-1);
		local_radio_init();
		radio_position = 0;
	}
}

uint8_t radio_recieve(){
	//If the radio is trying to tell us something, listen
	if(PINB & _BV(PB1)){
		hal_reset_flags();
		hal_isr_event_handler();
	}
	//If there is new data, read it
	if(hal_get_rx_start_flag()){
		hal_frame_read(&frame);
		//Only pay atention if the CRC is good
		if(frame.crc){
			return 1;
		}
	}
	return 0;
}

int radio_setup() {
    uint8_t ret;

    //Configure PinB1 as an input.
	//It's high if there is an interupt waiting
	DDRB &= ~_BV(PB1);
	
    ret = local_radio_init();
    if(ret != SUCCESS) {
        err_blinkout(ret);
		return ret;
    }
	
	//Enable the LED Outs on the Radio
	uint8_t temp = hal_register_read(TRX_CTRL_1) | PA_EXT_EN;
	hal_register_write(TRX_CTRL_1,temp);
	
	radio_position = 0;
	
    radio_led_off();
	return ret;
}

//These are stripped down/stolen functions from the radio library
int local_radio_init() {
    tat_status_t ret;
	static uint8_t radio_channel;
	static uint16_t radio_pan;
	static uint16_t radio_addr;

#ifndef RADIO_LED_SUPPRESS
    RADIO_LED1_DDR |= (1<<RADIO_LED1);
#if(RADIO_LED_HAVE_DUAL)
    RADIO_LED2_DDR |= (1<<RADIO_LED2);
#endif
    radio_led_off();
#endif
    
    // Initialize the radio device.
    ret = tat_init();
    if(ret != TAT_SUCCESS) {
        return ERR_TAT_INIT_FAIL;
    }

    /*
    // Tell the radio to give us the full 16MHz clock (we care more about performance
    // than about power)
    */

    // We want automatic transmit CRCs
    tat_use_auto_tx_crc(true);

    // Set up radio channel
    radio_channel = eeprom_read_byte(EEPROM_CHAN + EEPROM_CONFIG_OFFSET);
    if(radio_channel < 11 || radio_channel > 26)
        return ERR_INVALID_CHANNEL;
    ret = tat_set_operating_channel(radio_channel);
    if(ret != TAT_SUCCESS)
        return ERR_SETUP_RADIO_FAIL;

    // Set addresses
    radio_pan = eeprom_read_word(EEPROM_PAN + EEPROM_CONFIG_OFFSET);
    radio_addr = eeprom_read_word(EEPROM_ADDR + EEPROM_CONFIG_OFFSET);
    tat_set_short_address(radio_addr);
    tat_set_pan_id(radio_pan);
    tat_set_device_role(false);

    // Set up CCA
    tat_configure_csma(234, 0xE2);
    
    // Initialize the radio receive queue
    //rx_flag = true;

    // Set up the trx_end callback
    //hal_set_trx_end_event_handler(radio_trx_end_handler);

    ret = local_tat_set_trx_state(RX_AACK_ON);
    if(ret != TAT_SUCCESS)
        return ERR_RX_STATE_FAIL;

    return 0;
}

tat_status_t local_tat_set_trx_state( uint8_t new_state ){
    
    /*Check function paramter and current state of the radio transceiver.*/
    if (!((new_state == TRX_OFF ) || (new_state == RX_ON) || (new_state == PLL_ON) || 
        (new_state == RX_AACK_ON ) || (new_state == TX_ARET_ON ))) { 
            
        return TAT_INVALID_ARGUMENT; 
    }
    
    //if (is_sleeping( ) == true) { return TAT_WRONG_STATE; }
    
    uint8_t original_state = tat_get_trx_state( );
    
    if ((original_state == BUSY_RX ) || (original_state == BUSY_TX) || 
        (original_state == BUSY_RX_AACK) || (original_state == BUSY_TX_ARET)) { 
        return TAT_BUSY_STATE;
    }
    
    if (new_state == original_state) { return TAT_SUCCESS; }
                        
    //At this point it is clear that the requested new_state is:
    //TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON or TX_ARET_ON.
                
    //The radio transceiver can be in one of the following states:
    //TRX_OFF, RX_ON, PLL_ON, RX_AACK_ON, TX_ARET_ON.
    if( new_state == TRX_OFF ){
        tat_reset_state_machine( ); //Go to TRX_OFF from any state.
    } else {
        
        //It is not allowed to go from RX_AACK_ON or TX_AACK_ON and directly to
        //TX_AACK_ON or RX_AACK_ON respectively. Need to go via RX_ON or PLL_ON.
        if ((new_state == TX_ARET_ON) && (original_state == RX_AACK_ON)) {
            
            //First do intermediate state transition to PLL_ON, then to TX_ARET_ON.
            //The final state transition to TX_ARET_ON is handled after the if-else if.
            hal_subregister_write( SR_TRX_CMD, PLL_ON );
            delay_us( TIME_STATE_TRANSITION_PLL_ACTIVE );
        } else if ((new_state == RX_AACK_ON) && (original_state == TX_ARET_ON)) {
            
            //First do intermediate state transition to RX_ON, then to RX_AACK_ON.
            //The final state transition to RX_AACK_ON is handled after the if-else if.
            hal_subregister_write( SR_TRX_CMD, RX_ON );
            delay_us( TIME_STATE_TRANSITION_PLL_ACTIVE );
        }
            
        //Any other state transition can be done directly.    
        hal_subregister_write( SR_TRX_CMD, new_state );
        
        //When the PLL is active most states can be reached in 1us. However, from
        //TRX_OFF the PLL needs time to activate.
        if (original_state == TRX_OFF) {
            delay_us( TIME_TRX_OFF_TO_PLL_ACTIVE );
        } else {
            delay_us( TIME_STATE_TRANSITION_PLL_ACTIVE );
        } // end: if (original_state == TRX_OFF) ...
    } // end: if( new_state == TRX_OFF ) ...
        
    /*Verify state transition.*/
    tat_status_t set_state_status = TAT_TIMED_OUT;
    
    if( tat_get_trx_state( ) == new_state ){ set_state_status = TAT_SUCCESS; }
    
    return set_state_status;
}


void send_data( uint8_t frame_length, uint8_t *frame){ 
    
    /*Do sanity check on function parameters and current state.*/
    if ((frame_length > RF230_MAX_TX_FRAME_LENGTH) || 
        (frame_length < 3)) { 
        return; 
    }
    hal_clear_trx_end_flag( );
    //Write data to the frame buffer.
    hal_frame_write( frame, frame_length );
	/*Do initial frame transmission.*/
    hal_set_slptr_high( );
    hal_set_slptr_low( );
	//Wait for TRX_END interrupt.
	while (hal_get_trx_end_flag( ) == 0) {
		hal_isr_event_handler();
	}                                      
}
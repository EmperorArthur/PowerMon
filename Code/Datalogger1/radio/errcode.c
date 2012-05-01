#include <avr/io.h>
#include <util/delay.h>
#include "errcode.h"
#include "led.h"

/**
 * Used to indicate an error condition by blinking out error codes on the red LED channel
 * and then halting.  Tries to make sure that LED pins are configured as outputs and under
 * GPIO and not PWM control.
 */
void err_blinkout(uint8_t err) {
    uint8_t i;

    // force LEDs into GPIO control and turn all off
   
    RADIO_LED1_DDR |= (1<<RADIO_LED1);
    radio_led_off();

    _delay_ms(300);
    for(i=0; i<err; i++) {
        radio_led_on();
        _delay_ms(300);
        radio_led_off();
        _delay_ms(300);
    }

    // spin forever
    //for(;;) ;
}


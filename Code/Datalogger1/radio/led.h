#include "pinconfig.h"

#if(RADIO_LED_ACTIVE_LOW)
#   define RADIO_LED_ON_HAL(port, pin)  port &= ~(1<<pin)
#   define RADIO_LED_OFF_HAL(port, pin) port |=  (1<<pin)
#else
#   define RADIO_LED_ON_HAL(port, pin)  port |=  (1<<pin)
#   define RADIO_LED_OFF_HAL(port, pin) port &= ~(1<<pin)
#endif

#if(RADIO_LED_HAVE_DUAL)
#   define radio_led_tx_on() RADIO_LED_ON_HAL(RADIO_LED1_PORT, RADIO_LED1)
#   define radio_led_tx_off() RADIO_LED_OFF_HAL(RADIO_LED1_PORT, RADIO_LED1)
#   define radio_led_rx_on() RADIO_LED_ON_HAL(RADIO_LED2_PORT, RADIO_LED2)
#   define radio_led_rx_off() RADIO_LED_OFF_HAL(RADIO_LED2_PORT, RADIO_LED2)
#   define radio_led_on() { \
        radio_led_tx_on(); \
        radio_led_rx_on(); \
    }
#   define radio_led_off() { \
        radio_led_tx_off(); \
        radio_led_rx_off(); \
    }
#else
#   define radio_led_on()  RADIO_LED_ON_HAL(RADIO_LED1_PORT, RADIO_LED1)
#   define radio_led_off() RADIO_LED_OFF_HAL(RADIO_LED1_PORT, RADIO_LED1)
#   define radio_led_rx_on() radio_led_on()
#   define radio_led_rx_off() radio_led_off()
#   define radio_led_tx_on() radio_led_on()
#   define radio_led_tx_off() radio_led_off()
#endif




#ifndef __radio_uart_h_
#define __radio_uart_h_

#include "radio.h"
#include <stdio.h>



static uint8_t radio_buffer[129];
static uint8_t radio_position;
hal_rx_frame_t frame;
void radio_putchar_f(char c, FILE *stream);
void radio_putchar(char c);
uint8_t radio_recieve();
void radio_transmit();
uint8_t send(uint8_t buf[],uint8_t length);
int radio_setup();
void send_data( uint8_t frame_length, uint8_t *frame);
tat_status_t local_tat_set_trx_state( uint8_t new_state );
int local_radio_init();
#endif
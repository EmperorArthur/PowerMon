/****************************************************************************
Firmware for Finger v2 (Electric Field Sensing Board)

Brian Mayton <bmayton@cs.washington.edu>
(c) Intel Research Seattle, October 2008

This file handles the UART for RS232 communication, including buffered
reads.
****************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

#define BAUDRATE 9600        //The baudrate that we want to use
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)    //The formula that does all the required maths

// UART buffers
volatile char rxbuf[64];
volatile uint8_t rdptr = 0;
volatile uint8_t wrptr = 0;

/**
 * Set up the UART
 */
void uart_setup() {
	//UBRR0H = 0;
	//UBRR0L = 16;
	UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);
	//UCSR0A |= (1<<U2X0);			//Double the baud rate
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);

}

void uart_putchar(uint8_t c) {
	while( ! (UCSR0A & (1<<UDRE0)) );
	UDR0 = c;
}

int uart_putchar_f(char c, FILE *stream) {
    if(c == '\n') uart_putchar_f('\r', stream);
	while( ! (UCSR0A & (1<<UDRE0)) );
	UDR0 = c;
    return 0;
}

void uart_putdata(uint8_t *data, uint8_t count) {
	int i;
	for(i=0; i<count; i++) uart_putchar(data[i]);
}

uint8_t uart_data_available() {
    return !(rdptr == wrptr);
}

int16_t uart_getchar(uint8_t blocking) {
	char c;
    if(blocking)
        while(rdptr == wrptr);
    else
        if(rdptr == wrptr) return -1;
	c = rxbuf[rdptr];
	rdptr = (rdptr + 1) & 63;
	return c;
}

int uart_getchar_f(FILE *stream) {
    char c;
    while(rdptr == wrptr);
    c = rxbuf[rdptr];
    rdptr = (rdptr + 1) & 63;
    return c;
}

/**
 * UART Receive ISR
 */
ISR(USART_RX_vect) {
	rxbuf[wrptr] = UDR0;
    uart_putchar(rxbuf[wrptr]);	//Echo the charachter back (This is a good way of knowing if the UART is working)
	wrptr = (wrptr + 1) & 63;
	if(wrptr == rdptr) rdptr = (rdptr + 1) & 63;
}

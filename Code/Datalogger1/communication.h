//Radio and UART communication functions
//Copyright Arthur Moore 2012
//GPL V3
#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <avr/pgmspace.h>
#ifdef RADIOOUT
#include "radio/radio-uart.h"
#endif


void SpaceToZero(char* str,int length);	//This converts all spaces in a string to zeros
void cprint(char * str);				//This prints out a c string
//This lets me store pure strings in flash instead of data.
#define sprint(string) nprintf(PSTR(string))
void nprintf (PGM_P s);					//Never use this function.  Always use the macro above
int communication_setup();				//Set up both the UART and the Radio

#endif
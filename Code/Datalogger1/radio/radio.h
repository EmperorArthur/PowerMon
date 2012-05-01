#ifndef _radio_h_
#define _radio_h_

#include "compiler.h"
#include "hal.h"
#include "tat.h"
#include "at86rf230_registermap.h"

#define RX_FIFO_SIZE 4

int radio_init(void);

void radio_trx_end_handler(uint32_t timestamp);

uint8_t radio_receive_frame(void **data, uint8_t *lqi, uint8_t *ed);
void radio_receive_frame_done(void);

uint8_t radio_receive_frame_copy(void *data, uint8_t *lqi, uint8_t *ed, uint8_t max_length);

uint8_t radio_send_frame(uint8_t length, void *data);

void radio_set_pan(uint16_t pan, uint8_t nonvolatile);
void radio_set_addr(uint16_t addr, uint8_t nonvolatile);
uint8_t radio_set_chan(uint8_t chan, uint8_t nonvolatile);
uint8_t radio_set_txpower(uint8_t power);

void radio_off(void);
void radio_sleep(void);
void radio_wake(void);

int radio_set_clock_output(uint8_t speed);

uint16_t radio_get_pan(void);
uint16_t radio_get_addr(void);
uint8_t radio_get_chan(void);
uint8_t radio_get_txpower(void);

uint8_t radio_get_rx_activity_indicator(void);

#endif // _radio_h_


#ifndef _errcode_h_
#define _errcode_h_

#include <stdint.h>

#define SUCCESS 0

#define ERR_TAT_INIT_FAIL 1
#define ERR_TAT_CLKM_FAIL 2
#define ERR_RX_STATE_FAIL 3
#define ERR_INVALID_CHANNEL 4
#define ERR_SETUP_RADIO_FAIL 5
#define ERR_RADIO_NOT_INITIALIZED 6

void err_blinkout(uint8_t err);

#endif // _errcode_h_


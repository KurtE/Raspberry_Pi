/*
 * Author: Brendan Le Foll <brendan.le.foll@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mraa.h"
#include "math.h"
#include <unistd.h>
#define ADDRESS 0x60 //defines address of compass

#define MAX_BUFFER_LENGTH 6

int
main(int argc, char **argv)
{
    mraa_init();
    uint16_t direction = 0;
    uint8_t rx_tx_buf[MAX_BUFFER_LENGTH];

    mraa_i2c_context i2c;
    i2c = mraa_i2c_init(0);

    mraa_i2c_address(i2c, ADDRESS);
    uint8_t bVer = mraa_i2c_read_byte_data(i2c, 0);   // read one byte
    printf("CMPS03 Version: %d\n\r", bVer);
    

    for(;;) {
        mraa_i2c_address(i2c, ADDRESS);
        mraa_i2c_write_byte(i2c, 2);    // start read at byte 2
 
        mraa_i2c_address(i2c, ADDRESS);
        mraa_i2c_read(i2c, rx_tx_buf, 2);
        direction = (rx_tx_buf[0] << 8) + rx_tx_buf[1];
//        direction = mraa_i2c_read_word_data(i2c,  2);
        printf("Heading : %d\n", direction) ;
        usleep(250000);
    }
}

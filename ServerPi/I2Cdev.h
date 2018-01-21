// I2Cdev library collection - Main I2C device class
// Abstracts bit and byte I2C R/W functions into a convenient class
// RaspberryPi bcm2835 library port: bcm2835 library available at http://www.airspayce.com/mikem/bcm2835/index.html
// Based on Arduino's I2Cdev by Jeff Rowberg <jeff@rowberg.net>
//

/* ============================================
I2Cdev device library code is placed under the MIT license

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef _I2CDEV_H_
#define _I2CDEV_H_

#include <math.h> // required for BMP180
#include <stdlib.h> // required for MPU6060
#include <string.h> // required for MPU6060


#define set_I2C_pins  false  
/* used to boolean for setting RPi I2C pins P1-03 (SDA) and P1-05 (SCL) to alternate function ALT0, which enables those pins for I2C interface. 
   setI2Cpin should be false, if the I2C are already configured in alt mode ... */

#define i2c_baudrate 400000
//uint32_t i2c_baudrate = 400000 ; //400 kHz, 

class I2Cdev {
 public:
        I2Cdev();

        static void initialize();
        static void enable(bool isEnabled);

        static int8_t readBit(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitNum, u_int8_t *data);
        //TODO static int8_t readBitW(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitNum, uint16_t *data);
        static int8_t readBits(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitStart, u_int8_t length, u_int8_t *data);
        //TODO static int8_t readBitsW(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitStart, u_int8_t length, uint16_t *data);
        static int8_t readByte(u_int8_t devAddr, u_int8_t regAddr, u_int8_t *data);
        static int8_t readWord(u_int8_t devAddr, u_int8_t regAddr, u_int16_t *data);
        static int8_t readBytes(u_int8_t devAddr, u_int8_t regAddr, u_int8_t length, u_int8_t *data);
        static int8_t readWords(u_int8_t devAddr, u_int8_t regAddr, u_int8_t length, u_int16_t *data);

        static bool writeBit(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitNum, u_int8_t data);
        //TODO static bool writeBitW(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitNum, uint16_t data);
        static bool writeBits(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitStart, u_int8_t length, u_int8_t data);
        //TODO static bool writeBitsW(u_int8_t devAddr, u_int8_t regAddr, u_int8_t bitStart, u_int8_t length, uint16_t data);
        static bool writeByte(u_int8_t devAddr, u_int8_t regAddr, u_int8_t data);
        static bool writeWord(u_int8_t devAddr, u_int8_t regAddr, u_int16_t data);
        static bool writeBytes(u_int8_t devAddr, u_int8_t regAddr, u_int8_t length, u_int8_t *data);
        static bool writeWords(u_int8_t devAddr, u_int8_t regAddr, u_int8_t length, u_int16_t *data);
};

#endif /* _I2CDEV_H_ */

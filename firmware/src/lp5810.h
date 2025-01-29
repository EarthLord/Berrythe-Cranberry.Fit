/*
 * Copyright (c) 2025, Cranberry Fit (Empower Digital Health Pvt Ltd)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  lp5810.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Driver to use LP5810 LED driver via I2C
 */


#ifndef LP5810_H_
#define LP5810_H_

 #include "stdint.h"
 #include "stdbool.h"

#define LP5810_AUTO_MODE                (0)

/**
* @brief Starts the 4,7,8 breathing cycle
*   based on blue, green and red leds connected
*   to OUT0, OUT1 and OUT2 respectively.
*/
void lp5810_start_sequence(void);

#if (0 == LP5810_AUTO_MODE)
/**
* @brief API to be used in manual mode to set PWM
* @param Blue, green, red values ranges from 0 to 0xFF
*/
void lp5810_set_manual_bgr_pwm(uint8_t blue, uint8_t green, uint8_t red);
#endif

/**
* @brief Sets enable state of LP5810 by writing
*       chip_en (bit 0) of Chip_EN (register 0)
* @param true to enable, false to disable
*/
void lp5810_set_chip_en(bool chip_en);


#endif /* LP5810_H_ */

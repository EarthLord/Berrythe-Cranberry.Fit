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
void lp5810_start_478_sequence(void);

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

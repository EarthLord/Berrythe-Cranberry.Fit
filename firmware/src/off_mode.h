/*
 *  off_mode.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Module to handle off mode where the night
 *      yellow LEDs and breathe RGB LEDs are turned off
 *      by turning off MOSFET and LP5810 respectively.
 *      After this MSPM0 MCU goes to shutdown mode after
 *      enabling wakeup via button (BTN pin) on the device. 
 */

#ifndef OFF_MODE_H_
#define OFF_MODE_H_

/**
 * Start the off mode. Can transition from any other mode. 
 */
void off_mode_initiate(void);

#endif /* OFF_MODE_H_ */

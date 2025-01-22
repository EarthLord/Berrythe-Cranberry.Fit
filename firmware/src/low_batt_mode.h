/*
 *  low_batt_mode.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Module to handle low battery mode when battery's 
 *      SoC drops. The device will flash red and go to OFF mode
 */

#ifndef LOW_BATT_MODE_H_
#define LOW_BATT_MODE_H_

/**
 * Start the low battery mode. Can transition from any other mode. 
 */
void low_batt_mode_initiate(void);

#endif /* LOW_BATT_MODE_H_ */

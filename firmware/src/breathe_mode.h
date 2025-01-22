/*
 *  breathe_mode.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Module to handle breathe mode where the RGB LED is
 *      cycled through Inhale for 4s (green), hold for 7s (purple)
 *      and exhale for 8s (blue)
 */

#ifndef BREATHE_MODE_MODE_H_
#define BREATHE_MODE_MODE_H_

/**
 * Start the breathe mode. Should be on boot as it's first mode
 *  unless battery is low.
 */
void breathe_mode_initiate(void);

#endif /* BREATHE_MODE_MODE_H_ */

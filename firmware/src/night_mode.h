/*
 *  night_mode.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Module to handle night mode where the yellow light
 *      is turned on at a certain intensity for at most 30 min
 */

#ifndef NIGHT_MODE_MODE_H_
#define NIGHT_MODE_MODE_H_

/**
 * Start the night mode. Should be called from breathe mode as
 * per the requirement of the device.
 */
void night_mode_initiate(void);

#endif /* NIGHT_MODE_MODE_H_ */

/*
 *  btn_handler.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Module that works with BTN GPIO pin and periodic
 *      timer to check when a valid button press occured.
 *      A valid button press occurs when at least there is
 *      a specified amount of time in three stages - namely
 *      released1, press and released2. 
 */

#ifndef BTN_HANDLER_H_
#define BTN_HANDLER_H_

 #include "stdint.h"
 #include "stdbool.h"

/**
* @brief Checks if a valid button press has occured.
*       If yes, this function also clears this flag.
* @return True if a valid button press has occured, else false
*/
bool btn_handler_check_clear_press(void);

/**
* @brief Needs to be called periodically to check for
*       button press.
*/
void btn_handler_periodic_handler(void);

#endif /* BTN_HANDLER_H_ */

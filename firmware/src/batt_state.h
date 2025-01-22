
/*
 *  batt_state.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Driver to use internal ADC to get battery's state 
 */


#ifndef BATT_STATE_H_
#define BATT_STATE_H_

typedef enum
{
    BATT_LOW,       //Below 3.5V & not charging
    BATT_NORMAL,    //Above 3.5V & not charging
    BATT_CHARGING,  //Battery is being charged
}batt_state_t;

//Make this BATT_STATE_CHECK_PERIOD_S value a power of 2 to avoid division
#define BATT_STATE_CHECK_PERIOD_S           (16)
#define BATT_STATE_CHECK_PERIOD_MS          (BATT_STATE_CHECK_PERIOD_S*1000)
#define BATT_STATE_CHECK_COUNT              (BATT_STATE_CHECK_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS) 

/**
* @brief Gets the current state of battery based on voltages 
*       of battery and USB input for charger
*/
batt_state_t batt_state_get(void);

#endif /* BATT_STATE_H_ */



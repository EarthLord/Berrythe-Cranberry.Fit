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

//Make this BATT_STATE_CHECK_PERIOD_MS value a power of 2 to avoid division
#define BATT_STATE_CHECK_PERIOD_MS          (16384)
#define BATT_STATE_CHECK_COUNT              (BATT_STATE_CHECK_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS) 

/**
* @brief Gets the current state of battery based on voltages 
*       of battery and USB input for charger
*/
batt_state_t batt_state_get(void);

#endif /* BATT_STATE_H_ */



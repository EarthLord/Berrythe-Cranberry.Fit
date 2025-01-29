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

#include "night_mode.h"
#include "off_mode.h"
#include "periodic_timer.h"
#include "batt_state.h"
#include "low_batt_mode.h"
#include "tfp_printf.h"
#include "btn_handler.h"
#include "lp5810.h"

#define NIGHT_MODE_TIMEOUT_PERIOD_MIN       (30)
#define NIGHT_MODE_TIMEOUT_PERIOD_MS        (NIGHT_MODE_TIMEOUT_PERIOD_MIN*60*1000)
#define NIGHT_MODE_TIMEOUT_COUNT            (NIGHT_MODE_TIMEOUT_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS)

#define PWM_COUNTER_RELOAD_VALUE            (9999)
#define PWM_DUTY_PERCENT_COUNTER_VALUE(x)   (x*((PWM_COUNTER_RELOAD_VALUE+1)/100))

void night_periodic_handler(void)
{
    static uint32_t timeout_count = NIGHT_MODE_TIMEOUT_COUNT;

    //Handle button press to goto NIGHT mode
    bool btn_press = btn_handler_check_clear_press();
    if(true == btn_press)
    {
        off_mode_initiate();
    }

    //Handle timeout of NIGHT mode
    if(0 == timeout_count)
    {
        off_mode_initiate();
    }
    else 
    {
        timeout_count--;
    }

    //Handle if battery runs out
    if(0 == (timeout_count % BATT_STATE_CHECK_COUNT))
    {
        batt_state_t batt_state = batt_state_get();

        if(BATT_LOW == batt_state)
        {
            low_batt_mode_initiate();
        }//No else, continue in current mode
    }//No else
}

void night_mode_initiate(void)
{
    //Turn off LP5810
    lp5810_set_chip_en(false);

    //Turn on the PWM for the night yellow light
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 
            PWM_DUTY_PERCENT_COUNTER_VALUE(10), DL_TIMER_CC_1_INDEX);

    periodic_timer_set_second_callback(night_periodic_handler);

    tfp_printf("NMODE\r\n");
}
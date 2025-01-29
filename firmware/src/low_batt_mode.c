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

#include "low_batt_mode.h"
#include "off_mode.h"
#include "periodic_timer.h"
#include "lp5810.h"
#include "tfp_printf.h"

#define LOW_BATT_COUNT_PERIOD_MS(x)         (x/PERIODIC_TIMER_PERIOD_MS)

#define PERCENT_TO_HEX(x)                   ((x*255)/100)

typedef enum
{
    RED_RISE,
    RED_FALL,
    LOW_BATT_SEQ_MAX
}low_batt_seq_t;

const static uint16_t low_batt_seq_len_count[LOW_BATT_SEQ_MAX] = 
{
    LOW_BATT_COUNT_PERIOD_MS(500),       //RED_RISE,
    LOW_BATT_COUNT_PERIOD_MS(3000),      //RED_FALL,
};

const static uint8_t low_batt_pwm_value[LOW_BATT_SEQ_MAX+1] = 
{
    //Red
    PERCENT_TO_HEX(000),  //RED_RISE,
    PERCENT_TO_HEX(100),  //RED_FALL,
    PERCENT_TO_HEX(000),  //Goes back,
};

bool low_batt_set_seq_led_value(void)
{
    static low_batt_seq_t low_batt_seq = RED_RISE;
    static uint16_t seq_count = 0;
    uint8_t red_value;

    if(low_batt_seq_len_count[low_batt_seq] == seq_count)
    {
        seq_count = 0;
        low_batt_seq++;
        if(LOW_BATT_SEQ_MAX == low_batt_seq)
        {
            return false;
        }//No else
    }//No else

    seq_count++;

    //interpolation = value1 + (value2 - value2)*(interval)
    uint8_t value1 = low_batt_pwm_value[low_batt_seq];
    uint8_t value2 = low_batt_pwm_value[low_batt_seq+1];

    int16_t diff;
    if (value2 >= value1) 
    {
        diff = value2 - value1;
    } 
    else 
    {
        diff = -(int16_t)(value1 - value2);
    }

    red_value = (value1 + ((diff*seq_count)/low_batt_seq_len_count[low_batt_seq]));

    lp5810_set_manual_bgr_pwm(0, 0, red_value);

    return true;
}



void low_batt_periodic_handler(void)
{
    //End of low batt mode sequence
    if(false == low_batt_set_seq_led_value())
    {
        off_mode_initiate();
    }
}

void low_batt_mode_initiate(void)
{
    tfp_printf("LMODE\r\n");

    //Configure LP5810
    lp5810_start_sequence();

    periodic_timer_set_second_callback(low_batt_periodic_handler);

    //Turn off the PWM for the night yellow light
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0, DL_TIMER_CC_1_INDEX);
}
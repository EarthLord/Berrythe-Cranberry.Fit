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
 digraph G {
    nodesep=0.6
    "Mode specific\ninitialization" [shape="box"]
    "Set periodic\ntimer callback" [shape="box"]
    "Enter mode" -> "Mode specific\ninitialization"
    "Mode specific\ninitialization" -> "Set periodic\ntimer callback"
    "Set periodic\ntimer callback" -> Sleep
    "Is button\npressed" [shape="diamond"]
    "Is\nVbat<3.5V" [shape="diamond"]
    "Is mode\ntimed out" [shape="diamond"]
    "Mode specific\nprocess" [shape="box"]
    "Is button\npressed" -> "Start next\nmode" [label=" YES"]
    "Is\nVbat<3.5V" ->  "Go to low\nbattery mode" [label=" YES"]
    "Is mode\ntimed out" -> "Go to\nOFF mode" [label=" YES"]
    "Periodic timer\ncallback" -> "Is button\npressed"
    "Is button\npressed" -> "Is\nVbat<3.5V" [label=" NO"]
    "Is\nVbat<3.5V" ->  "Is mode\ntimed out" [label=" NO"]
    "Is mode\ntimed out" -> "Mode specific\nprocess"
    "Mode specific\nprocess" -> Sleep
}
 */

#include "breathe_mode.h"
#include "off_mode.h"
#include "periodic_timer.h"
#include "btn_handler.h"
#include "src/batt_state.h"
#include "src/night_mode.h"
#include "tfp_printf.h"
#include "batt_state.h"
#include "low_batt_mode.h"
#include "night_mode.h"
#include "lp5810.h"
#include "math.h"
#include <limits.h>

#define BREATHE_MODE_TIMEOUT_PERIOD_MIN     (10)
#define BREATHE_MODE_TIMEOUT_PERIOD_MS      (BREATHE_MODE_TIMEOUT_PERIOD_MIN*60*1000)
#define BREATHE_MODE_TIMEOUT_COUNT          (BREATHE_MODE_TIMEOUT_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS)

#define BREATHE_MODE_COUNT_PERIOD_MS(x)     (x/PERIODIC_TIMER_PERIOD_MS)

#define GAMMA                               (2.2)

#define PERCENT_TO_I16(x)                   ((x*INT16_MAX)/100)

#define ROUNDING_DIVISION_POSITIVE(n,d)     ((n+(d/2))/d)

typedef enum
{
    BLUE,
    GREEN,
    RED,
    COLOR_MAX,
}color_order_t;

typedef enum
{
    GREEN_RISE,
    GREEN_TO_PURPLE,
    PURPLE_STEADY,
    PURPLE_TO_BLUE,
    BLUE_SHARP_FALL,
    BLUE_FALL,
    BLUE_GREEN,
    BREATHE_SEQ_MAX
}breathe_seq_t;

const static uint16_t breathe_seq_len_count[BREATHE_SEQ_MAX+1] = 
{
    BREATHE_MODE_COUNT_PERIOD_MS(4000),       //GREEN_RISE,
    BREATHE_MODE_COUNT_PERIOD_MS(20),         //GREEN_TO_PURPLE,
    BREATHE_MODE_COUNT_PERIOD_MS(6900),       //PURPLE_STEADY,
    BREATHE_MODE_COUNT_PERIOD_MS(200),        //PURPLE_TO_BLUE,
    BREATHE_MODE_COUNT_PERIOD_MS(1000),       //BLUE_SHARP_FALL,
    BREATHE_MODE_COUNT_PERIOD_MS(6900),       //BLUE_FALL,
    BREATHE_MODE_COUNT_PERIOD_MS(20),         //BLUE_GREEN,
    0   //BREATHE_SEQ_MAX for 1st time logic to work
};

const static uint16_t breathe_pwm_percent[BREATHE_SEQ_MAX+1][COLOR_MAX] = 
{
    //Blue,             Green,              Red
    {PERCENT_TO_I16(  0),PERCENT_TO_I16( 10),PERCENT_TO_I16(  0)},  //GREEN_RISE,
    {PERCENT_TO_I16(  0),PERCENT_TO_I16( 80),PERCENT_TO_I16(  0)},  //GREEN_TO_PURPLE,
    {PERCENT_TO_I16(100),PERCENT_TO_I16( 60),PERCENT_TO_I16( 80)},  //PURPLE_STEADY,
    {PERCENT_TO_I16(100),PERCENT_TO_I16( 60),PERCENT_TO_I16( 80)},  //PURPLE_TO_BLUE,
    {PERCENT_TO_I16(100),PERCENT_TO_I16(  0),PERCENT_TO_I16(  0)},  //BLUE_SHARP_FALL,
    {PERCENT_TO_I16( 70),PERCENT_TO_I16(  0),PERCENT_TO_I16(  0)},  //BLUE_FALL,
    {PERCENT_TO_I16( 10),PERCENT_TO_I16(  0),PERCENT_TO_I16(  0)},  //BLUE_GREEN,
    {PERCENT_TO_I16(  0),PERCENT_TO_I16( 25),PERCENT_TO_I16(  0)},  //Goes back,
};

uint16_t gamma2_correction_i16(uint16_t value)
{
    //Optimized for Gamma value of 2 from
    //https://learn.adafruit.com/led-tricks-gamma-correction/the-longer-fix
    uint16_t result;
    uint32_t max = (1<<15);   
    uint32_t value_squared = ((uint32_t)value*value);
    result = ROUNDING_DIVISION_POSITIVE(value_squared,max);
    return (result);
}

void breathe_compute_set_seq_led_value(void)
{
    static breathe_seq_t breathe_seq = GREEN_RISE;
    static uint16_t seq_count = 0;
    uint8_t colors[COLOR_MAX];
    static int16_t diff[COLOR_MAX] = 
    {
        (breathe_pwm_percent[GREEN_TO_PURPLE][BLUE] - breathe_pwm_percent[GREEN_RISE][BLUE]),
        (breathe_pwm_percent[GREEN_TO_PURPLE][GREEN] - breathe_pwm_percent[GREEN_RISE][GREEN]),
        (breathe_pwm_percent[GREEN_TO_PURPLE][RED] - breathe_pwm_percent[GREEN_RISE][RED]),
    };

    if(breathe_seq_len_count[breathe_seq] == seq_count)
    {
        seq_count = 0;
        breathe_seq++;
        if(BREATHE_SEQ_MAX == breathe_seq)
        {
            breathe_seq = GREEN_RISE;
        }//No else

        //Difference doesn't change for the sequence
        for(uint32_t i = 0; i<COLOR_MAX; i++)
        {
            uint16_t value1 = breathe_pwm_percent[breathe_seq][i];
            uint16_t value2 = breathe_pwm_percent[breathe_seq+1][i];

            if (value2 >= value1) 
            {
                diff[i] = value2 - value1;
            } 
            else 
            {
                diff[i] = -(int16_t)(value1 - value2);
            }
        }
    }//No else

    seq_count++;

    //Linear interpolation
    for(uint32_t i = 0; i<COLOR_MAX; i++)
    {
        //interpolation = value1 + (value2 - value2)*(interval passed)
        uint16_t interpolate = 
                (breathe_pwm_percent[breathe_seq][i] + 
                ((diff[i]*seq_count)/breathe_seq_len_count[breathe_seq]));

        //tfp_printf("%d,",interpolate>>7);

        interpolate = gamma2_correction_i16(interpolate);

        //Max value of breathe_pwm_percent is 15 bit, making it 8 bit
        colors[i] = interpolate >> 7;
        //tfp_printf("%d,",colors[i]);
    }

    //tfp_printf("\r\n");

    lp5810_set_manual_bgr_pwm(colors[BLUE], colors[GREEN], colors[RED]);
}

/**
*/
void breathe_periodic_handler(void)
{
    static uint32_t timeout_count = BREATHE_MODE_TIMEOUT_COUNT;

    breathe_compute_set_seq_led_value();

    //Handle button press to goto NIGHT mode
    bool btn_press = btn_handler_check_clear_press();
    if(true == btn_press)
    {
        night_mode_initiate();
    }
    //Handle timeout of breathe mode
    //Don't have to reset for next time since MCU will reset
    if(0 == timeout_count)
    {
        tfp_printf("OFF\r\n");
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

void breathe_mode_initiate(void)
{
    tfp_printf("BMODE\r\n");

    //Configure LP5810
    lp5810_start_sequence();

    periodic_timer_set_second_callback(breathe_periodic_handler);
}

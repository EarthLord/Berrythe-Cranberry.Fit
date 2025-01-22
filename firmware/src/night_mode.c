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
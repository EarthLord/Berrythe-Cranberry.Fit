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
    lp5810_start_478_sequence();

    periodic_timer_set_second_callback(low_batt_periodic_handler);

    //Turn off the PWM for the night yellow light
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0, DL_TIMER_CC_1_INDEX);
}
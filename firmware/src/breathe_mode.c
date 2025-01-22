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

#define BREATHE_MODE_TIMEOUT_PERIOD_MIN     (10)
#define BREATHE_MODE_TIMEOUT_PERIOD_MS      (BREATHE_MODE_TIMEOUT_PERIOD_MIN*60*1000)
//#define BREATHE_MODE_TIMEOUT_COUNT          (BREATHE_MODE_TIMEOUT_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS)

#define BREATHE_MODE_TIMEOUT_COUNT          (0x80000000)

#define BREATHE_MODE_COUNT_PERIOD_MS(x)     (x/PERIODIC_TIMER_PERIOD_MS)

#define PERCENT_TO_HEX(x)                   ((x*255)/100)

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
    BLUE_FALL,
    BLUE_GREEN,
    BREATHE_SEQ_MAX
}breathe_seq_t;

const static uint16_t breathe_seq_len_count[BREATHE_SEQ_MAX] = 
{
    BREATHE_MODE_COUNT_PERIOD_MS(4000),       //GREEN_RISE,
    BREATHE_MODE_COUNT_PERIOD_MS(20),         //GREEN_TO_PURPLE,
    BREATHE_MODE_COUNT_PERIOD_MS(6900),       //PURPLE_STEADY,
    BREATHE_MODE_COUNT_PERIOD_MS(200),        //PURPLE_TO_BLUE,
    BREATHE_MODE_COUNT_PERIOD_MS(7900),       //BLUE_FALL,
    BREATHE_MODE_COUNT_PERIOD_MS(20),         //BLUE_GREEN,
};

const static uint8_t breathe_pwm_value[BREATHE_SEQ_MAX+1][COLOR_MAX] = 
{
    //Blue              Green               Red
    {PERCENT_TO_HEX(000),PERCENT_TO_HEX(025),PERCENT_TO_HEX(000)},  //GREEN_RISE,
    {PERCENT_TO_HEX(000),PERCENT_TO_HEX(100),PERCENT_TO_HEX(000)},  //GREEN_TO_PURPLE,
    {PERCENT_TO_HEX(100),PERCENT_TO_HEX(060),PERCENT_TO_HEX(80)},  //PURPLE_STEADY,
    {PERCENT_TO_HEX(100),PERCENT_TO_HEX(060),PERCENT_TO_HEX(80)},  //PURPLE_TO_BLUE,
    {PERCENT_TO_HEX(100),PERCENT_TO_HEX(000),PERCENT_TO_HEX(000)},  //BLUE_FALL,
    {PERCENT_TO_HEX(070),PERCENT_TO_HEX(000),PERCENT_TO_HEX(000)},  //BLUE_GREEN,
    {PERCENT_TO_HEX(000),PERCENT_TO_HEX(025),PERCENT_TO_HEX(000)},  //Goes back,
};

void breathe_compute_set_seq_led_value(void)
{
    static breathe_seq_t breathe_seq = GREEN_RISE;
    static uint16_t seq_count = 0;
    uint8_t colors[COLOR_MAX];

    if(breathe_seq_len_count[breathe_seq] == seq_count)
    {
        seq_count = 0;
        breathe_seq++;
        if(BREATHE_SEQ_MAX == breathe_seq)
        {
            breathe_seq = GREEN_RISE;
        }//No else
    }//No else

    seq_count++;

    //Linear interpolation
    for(uint32_t i = 0; i<COLOR_MAX; i++)
    {
        //interpolation = value1 + (value2 - value2)*(interval)
        uint8_t value1 = breathe_pwm_value[breathe_seq][i];
        uint8_t value2 = breathe_pwm_value[breathe_seq+1][i];

        int16_t diff;
        if (value2 >= value1) 
        {
            diff = value2 - value1;
        } 
        else 
        {
            diff = -(int16_t)(value1 - value2);
        }

        colors[i] = (value1 + ((diff*seq_count)/breathe_seq_len_count[breathe_seq]));
    }

    lp5810_set_manual_bgr_pwm(colors[BLUE], colors[GREEN], colors[RED]);
}

/**
*/
void breathe_periodic_handler(void)
{
    static uint32_t timeout_count = BREATHE_MODE_TIMEOUT_COUNT;

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

    breathe_compute_set_seq_led_value();
}

void breathe_mode_initiate(void)
{
    tfp_printf("BMODE\r\n");

    //Configure LP5810
    lp5810_start_478_sequence();

    periodic_timer_set_second_callback(breathe_periodic_handler);
}

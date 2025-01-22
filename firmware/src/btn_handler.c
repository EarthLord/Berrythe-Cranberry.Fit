#include "btn_handler.h"
#include "src/tfp_printf.h"
#include "ti_msp_dl_config.h"
#include "periodic_timer.h"

#define START_RELEASE_PERIOD_MS     (25)
#define PRESSED_PERIOD_MS           (25)
#define END_RELEASE_PERIOD_MS       (25)
#define VALID_PRESS_PERIOD_MS       (100)

#define START_RELEASE_COUNT         (START_RELEASE_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS)
#define PRESSED_COUNT               (PRESSED_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS)
#define END_RELEASE_COUNT           (END_RELEASE_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS)
#define VALID_PRESS_COUNT           (VALID_PRESS_PERIOD_MS/PERIODIC_TIMER_PERIOD_MS)

typedef enum
{
    START_RELEASE,
    PRESSED,
    END_RELEASE,
    VALID_PRESS
}btn_state_t;

static const bool BTN_PRESS_STATE_VALUE[] = 
{
    false,  //START_RELEASE  
    true,   //PRESSED
    false,  //END_RELEASE
};

static const uint32_t BTN_PRESS_STATE_COUNT[] = 
{
    START_RELEASE_COUNT,
    PRESSED_COUNT,
    END_RELEASE_COUNT,
    VALID_PRESS_COUNT,
};

static volatile btn_state_t g_btn_state = START_RELEASE;

void btn_time_check(btn_state_t current_state, bool btn_press)
{
    static uint32_t count = 0;

    if(BTN_PRESS_STATE_VALUE[current_state] == btn_press)
    {
        count++;
    }
    else 
    {
        //Handling short presses or releases - debouncing also
        count = 0;  
    }

    if(BTN_PRESS_STATE_COUNT[current_state] == count)
    {
        g_btn_state = current_state + 1;
        count = 0;      //Reset for next state
    }
    else 
    {
        //Nothing in else
    } 
}

void valid_press_handler(bool clear_valid_press)
{
    static uint32_t count = 0;
    if(true == clear_valid_press)
    {
        g_btn_state = START_RELEASE;
        count = 0;      //Reset for next valid press
        return ;
    }//No else since there is an early return
    
    if(BTN_PRESS_STATE_COUNT[VALID_PRESS] == count)
    {
        g_btn_state = START_RELEASE;
        count = 0;
    }
    else
    {
        count++;
    }
}

bool btn_handler_check_clear_press(void)
{
    bool ret_btn_pressed;

    if(VALID_PRESS == g_btn_state)
    {
        ret_btn_pressed = true;
        //Clears btn_state back
        valid_press_handler(true);
    }
    else 
    {
        ret_btn_pressed = false;
    }

    return ret_btn_pressed;
}

void btn_handler_periodic_handler(void)
{
    uint32_t pin_val = DL_GPIO_readPins(
        GPIO_GRP_0_PORT, GPIO_GRP_0_BTN_PIN);

    //Considering button press is active low
    bool btn_press = (0 == pin_val)?true:false;
#if 0
    tfp_printf("W%x ",pin_val);
#endif
    if(VALID_PRESS == g_btn_state)
    {
        valid_press_handler(false);
    }
    else    //START_RELEASE, PRESSED, END_RELEASE
    {
        btn_time_check(g_btn_state, btn_press);
    }
}

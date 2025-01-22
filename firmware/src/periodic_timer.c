#include "periodic_timer.h"
#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>

static void (*timer_first_callback)(void);
static void (*timer_second_callback)(void);

void periodic_timer_init(void)
{
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    DL_TimerG_startCounter(TIMER_0_INST);
}

void periodic_timer_set_first_callback(void (*callback)(void))
{
    timer_first_callback = callback;
}

void periodic_timer_set_second_callback(void (*callback)(void))
{
    timer_second_callback = callback;
}

void TIMER_0_INST_IRQHandler(void)
{
    if(NULL != timer_first_callback)
    {
        timer_first_callback();
    }

    if(NULL != timer_second_callback)
    {
        timer_second_callback();
    }
}
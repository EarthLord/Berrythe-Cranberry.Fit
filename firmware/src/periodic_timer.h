/*
 *  periodic_timer.h
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */

/**
 * @brief Driver to generate periodic callbacks using TIMG timer
 *          Assumption is a timer with TIMER0 instance is initialized 
 *          to appropriate time period and stared using syscfg with below code

 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_0_INST_LOAD_VALUE = (125 ms * 32768 Hz) - 1
 
static const DL_TimerG_TimerConfig gTIMER_0TimerConfig = {
    .period     = TIMER_0_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_0_init(void) {

    DL_TimerG_setClockConfig(TIMER_0_INST,
        (DL_TimerG_ClockConfig *) &gTIMER_0ClockConfig);

    DL_TimerG_initTimerMode(TIMER_0_INST,
        (DL_TimerG_TimerConfig *) &gTIMER_0TimerConfig);
    DL_TimerG_enableInterrupt(TIMER_0_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
	NVIC_SetPriority(TIMER_0_INST_INT_IRQN, 1);
    DL_TimerG_enableClock(TIMER_0_INST);
}

 * @{
 */

#ifndef PERIODIC_TIMER_H_
#define PERIODIC_TIMER_H_

#include "ti_msp_dl_config.h"

//To get the interval based on configuration of TIMER0
#define PERIODIC_TIMER_PERIOD_MS    (((TIMER_0_INST_LOAD_VALUE+1)*1000)/32768)

/**
 * Initialize and start the periodic timer
 */
void periodic_timer_init(void);

/**
 * Set first callback to be called periodically
 * @param callback 	Pointer to a function which needs to be called when the timer expires
 */
void periodic_timer_set_first_callback(void (*callback)(void));

/**
 * Set second callback to be called periodically
 * @param callback 	Pointer to a function which needs to be called when the timer expires
 */
void periodic_timer_set_second_callback(void (*callback)(void));


#endif /* PERIODIC_TIMER_H_ */
/**
 * @}
 */

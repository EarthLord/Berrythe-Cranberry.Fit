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

#include "off_mode.h"
#include "src/tfp_printf.h"
#include "ti_msp_dl_config.h"
#include "lp5810.h"

/**
* @brief Goes to go to SHUTDOWN mode, would reset on wakeup from BTN pin
*/
void goto_standby(void)
{
    DL_GPIO_clearInterruptStatus(GPIO_GRP_0_PORT, GPIO_GRP_0_BTN_PIN);
    NVIC_ClearPendingIRQ(GPIO_GRP_0_INT_IRQN);
    NVIC_EnableIRQ(GPIO_GRP_0_INT_IRQN);

    DL_SYSCTL_setPowerPolicySTANDBY1();

    __WFI();
}

void off_mode_turn_off_leds(void)
{
    //Switch off LED driver
    lp5810_set_chip_en(false);
    
    //Turn off timer used for PWM
    DL_TimerG_reset(PWM_0_INST);

    //Set the pin driving the MOSFET gate for night light as input
    //There's a pull down resistor on the PCB to turn off the LEDs
    DL_GPIO_initDigitalInputFeatures(GPIO_PWM_0_C1_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

/**
* @brief Transition to OFF mode by turning off all hardware,
*       enable wakeup from BTN pin and go to SHUTDOWN mode
*/
void off_mode_initiate(void)
{
    tfp_printf("OMODE\r\n");

    off_mode_turn_off_leds();

    //Turn on the PWM for the night yellow light
    //DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0, DL_TIMER_CC_1_INDEX);

    goto_standby();

    tfp_printf("after standby1\r\n");

    DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_POR);

    while(1)
    {
        //Shouldn't come here
    }
}

/**
* Interrupt enabled only after entering the OFF mode
*/
void GPIOA_IRQHandler(void)
{
    tfp_printf("Woke up ");
}
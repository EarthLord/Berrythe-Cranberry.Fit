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
 *  berrythe_main.c
 *
 *  Copyright (c) 2025, Cranberry Fit
 *  All rights reserved.
 */


#include "ti/driverlib/dl_common.h"
#include "ti_msp_dl_config.h"
#include "tfp_printf.h"
#include "stdint.h"
#include "periodic_timer.h"
#include "lp5810.h"
#include "low_batt_mode.h"
#include "batt_state.h"
#include "off_mode.h"
#include "night_mode.h"
#include "breathe_mode.h"
#include "btn_handler.h"

typedef enum
{
    MODE_OFF,
    MODE_BREATHE,
    MODE_NIGHT,
    MODE_LOW_BATT
}berrythe_modes_t;

char * reset_text(DL_SYSCTL_RESET_CAUSE reset_cause)
{
#if 1
    switch(reset_cause)
    {
        case DL_SYSCTL_RESET_CAUSE_NO_RESET: return("No Reset Since Last Read");
        case DL_SYSCTL_RESET_CAUSE_POR_HW_FAILURE: return("(VDD < POR- violation) or (PMU trim parity fault) or (SHUTDNSTOREx parity fault)");
        case DL_SYSCTL_RESET_CAUSE_POR_EXTERNAL_NRST: return("NRST pin reset (>1s)");
        case DL_SYSCTL_RESET_CAUSE_POR_SW_TRIGGERED: return("Software-trig POR");
        case DL_SYSCTL_RESET_CAUSE_BOR_SUPPLY_FAILURE: return("VDD < BOR- violation");
        case DL_SYSCTL_RESET_CAUSE_BOR_WAKE_FROM_SHUTDOWN: return("Wake from SHUTDOWN");
        case DL_SYSCTL_RESET_CAUSE_BOOTRST_NON_PMU_PARITY_FAULT: return("Non-PMU trim parity fault");
        case DL_SYSCTL_RESET_CAUSE_BOOTRST_CLOCK_FAULT: return("Fatal clock fault");
        case DL_SYSCTL_RESET_CAUSE_BOOTRST_SW_TRIGGERED: return("Software-trig BOOTRST");
        case DL_SYSCTL_RESET_CAUSE_BOOTRST_EXTERNAL_NRST: return("NRST pin reset (<1s)");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_BSL_EXIT: return("BSL exit");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_BSL_ENTRY: return("BSL entry");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_WWDT0_VIOLATION: return("WWDT0 violation");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_WWDT1_VIOLATION: return("WWDT1 violation");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_FLASH_ECC_ERROR: return("Uncorrectable flash ECC error");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_CPU_LOCKUP_VIOLATION: return("CPULOCK violation");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_DEBUG_TRIGGERED: return("Debug-trig SYSRST");
        case DL_SYSCTL_RESET_CAUSE_SYSRST_SW_TRIGGERED: return("Software-trig SYSRST");
        case DL_SYSCTL_RESET_CAUSE_CPURST_DEBUG_TRIGGERED: return("Debug-trig CPURST");
        case DL_SYSCTL_RESET_CAUSE_CPURST_SW_TRIGGERED: return("Software-trig CPURST");
        default: return("Error - wrong reset cause value");
    }
#else
    return("RST ");
#endif
}

void reset_cause_handler(void)
{
    volatile DL_SYSCTL_RESET_CAUSE reset_cause;
    reset_cause = DL_SYSCTL_getResetCause();

    printf("%s %x\r\n",reset_text(reset_cause),reset_cause);
}

int main(void)
{
    SYSCFG_DL_init();

    //These print also gives time for VREF to settle
    tfp_printf("\r\nLet's breathe\r\n");
    
    reset_cause_handler();

    periodic_timer_init();
    periodic_timer_set_first_callback(btn_handler_periodic_handler);

    /* Confirm VREF has settled before using ADC */
    while (DL_VREF_CTL1_READY_NOTRDY == DL_VREF_getStatus(VREF)){}

    batt_state_t batt_state = batt_state_get();

    tfp_printf("Batt %s\r\n",
        (BATT_LOW == batt_state)?"low":((BATT_NORMAL == batt_state)?"OK":"chrg"));

    if(BATT_LOW == batt_state)
    {
        low_batt_mode_initiate();
    }
    else 
    {
        breathe_mode_initiate();
    }

    while (1) 
    {
        __WFI();
    }
}

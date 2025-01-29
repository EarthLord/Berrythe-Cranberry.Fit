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

#include "batt_state.h"
#include "ti/driverlib/m0p/dl_core.h"
#include "ti_msp_dl_config.h"
#include "stdint.h"
#include "tfp_printf.h"

#define DELAY_1_US_COUNT        (CPUCLK_FREQ/1000000)
#define ADC_SAMPLING_DELAY_US   (450)

#define VBAT_CUTOFF_THRESHOLD_MV    (3500)
#define VIN_CUTOFF_THRESHOLD_MV     (4800)

//Voltage divider of 47k and 47k
//Voltage drops by a ratio of (47/94)
//Conversion to 2.5V reference 10 bit ADC value by
//THRESHOLD*(47/94)*(1024)/(2500)
#define VBAT_ADC_VALUE_CUTOFF       ((VBAT_CUTOFF_THRESHOLD_MV*128)/(625))

//Voltage divider of 5.1k and 47k
//Voltage drops by a ratio of (5.1/52.1)
//Conversion to 2.5V reference 10 bit ADC value by
//THRESHOLD*(5.1/52.1)*(1024)/(2500)
#define VIN_ADC_VALUE_CUTOFF       ((VIN_CUTOFF_THRESHOLD_MV*13056)/(325625))

batt_state_t batt_state_get(void)
{
    int16_t batt_value, usb_vin_value;

    /* Get calibrated ADC offset - workaround for ADC_ERR_06 */
    int16_t ADC_offset = 
        DL_ADC12_getADCOffsetCalibration(ADC12_0_ADCMEM_VBAT_REF_VOLTAGE_V);

    DL_ADC12_startConversion(ADC12_0_INST);
    
    delay_cycles(500*DELAY_1_US_COUNT);
    
    //This wasn't working, not sure why
#if 0
    while(ADC12_STATUS_BUSY_ACTIVE == 
            (DL_ADC12_getStatus(ADC12_0_INST) & ADC12_STATUS_BUSY_MASK)){}
#endif

    batt_value = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_VBAT);
    usb_vin_value = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_VUSB);

    DL_ADC12_enableConversions(ADC12_0_INST);

    batt_value = batt_value + ADC_offset;
    usb_vin_value = usb_vin_value + ADC_offset;

    tfp_printf("Conv %d, %d\r\n", (batt_value*625)/128, usb_vin_value);

    //Valid USB Vin voltage above 4.8V
    if(usb_vin_value > VIN_ADC_VALUE_CUTOFF)
    {
        return BATT_CHARGING;
    }
    //When not charging, battery voltage above 3.5V
    //corresponding to about 10% SoC
    else if (batt_value > VBAT_ADC_VALUE_CUTOFF)
    {
        return BATT_NORMAL;
    } else 
    {
        return BATT_LOW;
    }
}


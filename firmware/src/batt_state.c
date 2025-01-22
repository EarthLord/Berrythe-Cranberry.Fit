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
    int16_t ADC_offset = 0;
    //     DL_ADC12_getADCOffsetCalibration(ADC12_0_ADCMEM_VBAT_REF_VOLTAGE_V);

    DL_ADC12_startConversion(ADC12_0_INST);
    
    delay_cycles(500*DELAY_1_US_COUNT);
    
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


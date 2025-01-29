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

#include "lp5810.h"
#include "src/tfp_printf.h"
#include "ti_msp_dl_config.h"
#include "tfp_printf.h"
#include "string.h"

/* 'D' variant of LP5810 I2C address except all status registers  */
#define LP5810D_I2C_TARGET_ADDRESS      (0x5C)   //0b1011100

/* 'D' variant of LP5810 I2C address for 0x3xx registers, mainly status  */
#define LP5810D_I2C_TARGET_ADDRESS_0x3xx    (0x5F)   //0b1011111

/* Broadcast address for LP5810 - 8 bit data address*/
#define LP5810_I2C_BROADCAST_ADDRESS    (0x6C)   //0b1101100

/* Broadcast address for LP5810 - 10 bit data address for register 0x3xx */
#define LP5810_I2C_BROADCAST_ADDRESS_0x3xx  (0x6F)   //0b1101111

#define LED_BLUE_INTENSITY_PERCENTAGE   (100)
#define LED_GREEN_INTENSITY_PERCENTAGE  (75)
#define LED_RED_INTENSITY_PERCENTAGE    (100)

#define LED_BLUE_INTENSITY_DC_VALUE     ((LED_BLUE_INTENSITY_PERCENTAGE*255)/100)
#define LED_GREEN_INTENSITY_DC_VALUE    ((LED_GREEN_INTENSITY_PERCENTAGE*255)/100)
#define LED_RED_INTENSITY_DC_VALUE      ((LED_RED_INTENSITY_PERCENTAGE*255)/100)

#define LP5810_AUTO_CONTROL_START_ADRS  (0x80)

//Only comments are the default values on boot, so not writing to them
static const uint8_t lp5810_dev_config_3to5[] = 
{
    //Config 0 - max current set to 25.5 mA
    //Config 1 - PWM freq set to 24 kHz
    0x04,   //Config 3 address
#if LP5810_AUTO_MODE
    0x07,   //Config 3 - Enable auto mode for LED 0, 1, 2
#else
    0x00,   //Config 3 - Leave in manual mode for LED 0, 1, 2
#endif
    0x00,   //Config 4 - Reserved
    0x00,   //Config 5 - Disable exponential brightness curve for LED 0, 1, 2
    //Config 7 - All PWM phases are forward aligned
    //Config 11 - VSync is set to input and blank time is 1us
};

static const uint8_t lp5810_dev_config_12[] = 
{
    0x0D,   //Config 12 address
    0x0F,   //Config 12 - Enable LED open & short detection to shutdown current sink on both
            //Short detection threshold of 0.65 of VCC
};

static const uint8_t lp5810_led_en1[] = 
{
    0x20,   //LED EN1 address
    0x07,   //LED EN1 - Enable LED 0, 1 and 2
};

static const uint8_t lp5810_auto_dc[] = 
{
    0x50,   //Auto DC0 address
    LED_BLUE_INTENSITY_DC_VALUE,   //Auto DC0 percentage
    LED_GREEN_INTENSITY_DC_VALUE,  //Auto DC1 percentage
    LED_RED_INTENSITY_DC_VALUE,    //Auto DC2 percentage
};

static const uint8_t lp5810_manual_dc[] = 
{
    0x30,   //Manual DC0 address
    LED_BLUE_INTENSITY_DC_VALUE,   //Manual DC0 percentage
    LED_GREEN_INTENSITY_DC_VALUE,  //Manual DC1 percentage
    LED_RED_INTENSITY_DC_VALUE,    //Manual DC2 percentage
};

#if LP5810_AUTO_MODE
#if 0
static const uint8_t lp5810_auto_control[] = 
{
    0x00, //led0_pause_time
    0x1F, //led0_playback times
    0x00, //led0_aeu1/pwm1
    0x00, //led0_aeu1/pwm2
    0xFF, //led0_aeu1/pwm3
    0xFF, //led0_aeu1/pwm4
    0xFF, //led0_aeu1/pwm5
    0x2B, //led0_aeu1/slope_time1
    0x2E, //led0_aeu1/slope_time2
    0x00, //led0_aeu1/pt1
    0xFF, //led0_aeu2/pwm1
    0x00, //led0_aeu2/pwm2
    0x00, //led0_aeu2/pwm3
    0x00, //led0_aeu2/pwm4
    0x00, //led0_aeu2/pwm5
    0x0F, //led0_aeu2/slope_time1
    0x00, //led0_aeu2/slope_time2
    0x00, //led0_aeu2/pt1
    0x00, //led0_aeu3/pwm1
    0x00, //led0_aeu3/pwm2
    0x00, //led0_aeu3/pwm3
    0x00, //led0_aeu3/pwm4
    0x00, //led0_aeu3/pwm5
    0x00, //led0_aeu3/slope_time1
    0x00, //led0_aeu3/slope_time2
    0x00, //led0_aeu3/pt1
    0x00, //led1_pause time
    0x1F, //led1_playback times
    0x00, //led1_aeu1/pwm1
    0xFF, //led1_aeu1/pwm2
    0x00, //led1_aeu1/pwm3
    0x00, //led1_aeu1/pwm4
    0x00, //led1_aeu1/pwm5
    0x2B, //led1_aeu1/slope_time1
    0x2E, //led1_aeu1/slope_time2
    0x00, //led1_aeu1/pt1
    0x00, //led1_aeu2/pwm1
    0x00, //led1_aeu2/pwm2
    0x00, //led1_aeu2/pwm3
    0x00, //led1_aeu2/pwm4
    0x00, //led1_aeu2/pwm5
    0x0F, //led1_aeu2/slope_time1
    0x00, //led1_aeu2/slope_time2
    0x00, //led1_aeu2/pt1
    0x00, //led1_aeu3/pwm1
    0x00, //led1_aeu3/pwm2
    0x00, //led1_aeu3/pwm3
    0x00, //led1_aeu3/pwm4
    0x00, //led1_aeu3/pwm5
    0x00, //led1_aeu3/slope_time1
    0x00, //led1_aeu3/slope_time2
    0x00, //led1_aeu3/pt1
    0x00, //led2_pause time
    0x1F, //led2_playback times
    0x00, //led2_aeu1/pwm1
    0x00, //led2_aeu1/pwm2
    0xFF, //led2_aeu1/pwm3
    0xFF, //led2_aeu1/pwm4
    0x00, //led2_aeu1/pwm5
    0x2B, //led2_aeu1/slope_time1
    0x2E, //led2_aeu1/slope_time2
    0x00, //led2_aeu1/pt1
    0x00, //led2_aeu2/pwm1
    0x00, //led2_aeu2/pwm2
    0x00, //led2_aeu2/pwm3
    0x00, //led2_aeu2/pwm4
    0x00, //led2_aeu2/pwm5
    0x0F, //led2_aeu2/slope_time1
    0x00, //led2_aeu2/slope_time2
    0x00, //led2_aeu2/pt1
    0x00, //led2_aeu3/pwm1
    0x00, //led2_aeu3/pwm2
    0x00, //led2_aeu3/pwm3
    0x00, //led2_aeu3/pwm4
    0x00, //led2_aeu3/pwm5
    0x00, //led2_aeu3/slope_time1
    0x00, //led2_aeu3/slope_time2
    0x00, //led2_aeu3/pt1
    0x00, //led3_pause time
    0x00, //led3_playback times
    0x00, //led3_aeu1/pwm1
    0x00, //led3_aeu1/pwm2
    0x00, //led3_aeu1/pwm3
    0x00, //led3_aeu1/pwm4
    0x00, //led3_aeu1/pwm5
    0x00, //led3_aeu1/slope_time1
    0x00, //led3_aeu1/slope_time2
    0x00, //led3_aeu1/pt1
    0x00, //led3_aeu2/pwm1
    0x00, //led3_aeu2/pwm2
    0x00, //led3_aeu2/pwm3
    0x00, //led3_aeu2/pwm4
    0x00, //led3_aeu2/pwm5
    0x00, //led3_aeu2/slope_time1
    0x00, //led3_aeu2/slope_time2
    0x00, //led3_aeu2/pt1
    0x00, //led3_aeu3/pwm1
    0x00, //led3_aeu3/pwm2
    0x00, //led3_aeu3/pwm3
    0x00, //led3_aeu3/pwm4
    0x00, //led3_aeu3/pwm5
    0x00, //led3_aeu3/slope_time1
    0x00, //led3_aeu3/slope_time2
    0x00, //led3_aeu3/pt1
};
#else
static const uint8_t lp5810_auto_control[] = 
{
    0x00, //led0_pause_time
    0x1F, //led0_playback times
    0x00, //led0_aeu1/pwm1
    0x00, //led0_aeu1/pwm2
    0xFF, //led0_aeu1/pwm3
    0xFF, //led0_aeu1/pwm4
    0x00, //led0_aeu1/pwm5
    0x0A, //led0_aeu1/slope_time1
    0x0A, //led0_aeu1/slope_time2
    0x00, //led0_aeu1/pt1
    0x00, //led0_aeu2/pwm1
    0x00, //led0_aeu2/pwm2
    0xFF, //led0_aeu2/pwm3
    0xFF, //led0_aeu2/pwm4
    0x00, //led0_aeu2/pwm5
    0x07, //led0_aeu2/slope_time1
    0x07, //led0_aeu2/slope_time2
    0x00, //led0_aeu2/pt1
    0x00, //led0_aeu3/pwm1
    0x00, //led0_aeu3/pwm2
    0x00, //led0_aeu3/pwm3
    0x00, //led0_aeu3/pwm4
    0x00, //led0_aeu3/pwm5
    0x00, //led0_aeu3/slope_time1
    0x00, //led0_aeu3/slope_time2
    0x00, //led0_aeu3/pt1
 /*   
    0x00, //led1_pause time
    0x1F, //led1_playback times
    0x00, //led1_aeu1/pwm1
    0xFF, //led1_aeu1/pwm2
    0x00, //led1_aeu1/pwm3
    0x00, //led1_aeu1/pwm4
    0x00, //led1_aeu1/pwm5
    0x2B, //led1_aeu1/slope_time1
    0x2E, //led1_aeu1/slope_time2
    0x00, //led1_aeu1/pt1
    0x00, //led1_aeu2/pwm1
    0x00, //led1_aeu2/pwm2
    0x00, //led1_aeu2/pwm3
    0x00, //led1_aeu2/pwm4
    0x00, //led1_aeu2/pwm5
    0x0F, //led1_aeu2/slope_time1
    0x00, //led1_aeu2/slope_time2
    0x00, //led1_aeu2/pt1
    0x00, //led1_aeu3/pwm1
    0x00, //led1_aeu3/pwm2
    0x00, //led1_aeu3/pwm3
    0x00, //led1_aeu3/pwm4
    0x00, //led1_aeu3/pwm5
    0x00, //led1_aeu3/slope_time1
    0x00, //led1_aeu3/slope_time2
    0x00, //led1_aeu3/pt1
    0x00, //led2_pause time
    0x1F, //led2_playback times
    0x00, //led2_aeu1/pwm1
    0x00, //led2_aeu1/pwm2
    0xFF, //led2_aeu1/pwm3
    0xFF, //led2_aeu1/pwm4
    0x00, //led2_aeu1/pwm5
    0x2B, //led2_aeu1/slope_time1
    0x2E, //led2_aeu1/slope_time2
    0x00, //led2_aeu1/pt1
    0x00, //led2_aeu2/pwm1
    0x00, //led2_aeu2/pwm2
    0x00, //led2_aeu2/pwm3
    0x00, //led2_aeu2/pwm4
    0x00, //led2_aeu2/pwm5
    0x0F, //led2_aeu2/slope_time1
    0x00, //led2_aeu2/slope_time2
    0x00, //led2_aeu2/pt1
    0x00, //led2_aeu3/pwm1
    0x00, //led2_aeu3/pwm2
    0x00, //led2_aeu3/pwm3
    0x00, //led2_aeu3/pwm4
    0x00, //led2_aeu3/pwm5
    0x00, //led2_aeu3/slope_time1
    0x00, //led2_aeu3/slope_time2
    0x00, //led2_aeu3/pt1
    0x00, //led3_pause time
    0x00, //led3_playback times
    0x00, //led3_aeu1/pwm1
    0x00, //led3_aeu1/pwm2
    0x00, //led3_aeu1/pwm3
    0x00, //led3_aeu1/pwm4
    0x00, //led3_aeu1/pwm5
    0x00, //led3_aeu1/slope_time1
    0x00, //led3_aeu1/slope_time2
    0x00, //led3_aeu1/pt1
    0x00, //led3_aeu2/pwm1
    0x00, //led3_aeu2/pwm2
    0x00, //led3_aeu2/pwm3
    0x00, //led3_aeu2/pwm4
    0x00, //led3_aeu2/pwm5
    0x00, //led3_aeu2/slope_time1
    0x00, //led3_aeu2/slope_time2
    0x00, //led3_aeu2/pt1
    0x00, //led3_aeu3/pwm1
    0x00, //led3_aeu3/pwm2
    0x00, //led3_aeu3/pwm3
    0x00, //led3_aeu3/pwm4
    0x00, //led3_aeu3/pwm5
    0x00, //led3_aeu3/slope_time1
    0x00, //led3_aeu3/slope_time2
    0x00, //led3_aeu3/pt1
    */
};
#endif
#endif

/**
* @brief A blocking call that sends a 
* @warning As FIFO is of 4 bytes, len should be at most 4
*/
bool i2c_write(uint8_t i2c_adrs, uint8_t * data, uint32_t len)
{
#if 1
    if(0x40 != data[0])
    {
        tfp_printf("Wr %2x ", i2c_adrs);

        for(uint32_t i=0;i<len;i++)
        {
            tfp_printf("%2x ",data[i]);
        }

        tfp_printf("\r\n");
    }
#endif
    //Fill 8 bytes FIFO
    DL_I2C_fillControllerTXFIFO(I2C_0_INST, data, len);

    /* Wait for I2C to be Idle */
    while (!(
        DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)){}

    /* Send the packet to the controller.
     * This function will send Start + Stop automatically. */
    DL_I2C_startControllerTransfer(I2C_0_INST, i2c_adrs,
        DL_I2C_CONTROLLER_DIRECTION_TX, len);

    /* Poll until the Controller writes all bytes */
    while (DL_I2C_getControllerStatus(I2C_0_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS) {}

    /* Trap if there was an error */
    if (DL_I2C_getControllerStatus(I2C_0_INST) &
        DL_I2C_CONTROLLER_STATUS_ERROR) 
    {
        tfp_printf("error write\r\n");
        //__BKPT(0);
        return false;
    }
    
    /* Wait for I2C to be Idle */
    while (!(
        DL_I2C_getControllerStatus(I2C_0_INST) & DL_I2C_CONTROLLER_STATUS_IDLE)){}

    return true;
}

bool i2c_read(uint8_t i2c_adrs, uint8_t read_ptr, uint8_t * buffer, uint32_t len)
{
    //Send the address to read from before starting to read
    i2c_write(i2c_adrs, &read_ptr, sizeof(read_ptr));

    /* Send a read request to Target */
    DL_I2C_startControllerTransfer(I2C_0_INST, i2c_adrs,
        DL_I2C_CONTROLLER_DIRECTION_RX, len);

    /* Poll until the Controller writes all bytes */
    while (DL_I2C_getControllerStatus(I2C_0_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS) {}

    /* Trap if there was an error */
    if (DL_I2C_getControllerStatus(I2C_0_INST) &
        DL_I2C_CONTROLLER_STATUS_ERROR) 
    {
        __BKPT(0);
        return false;
    }

    /* Receive all bytes from target. */
    for (uint8_t i = 0; i < len; i++) {
        //Poll till length number of bytes are received
        while (DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST)){}

        buffer[i] = DL_I2C_receiveControllerData(I2C_0_INST);
    }

    return true;
}

void lp5810_set_chip_en(bool chip_en)
{
    uint8_t lp5810_chip_en_arr[] = {0x00, 0x00};

    lp5810_chip_en_arr[1] = (chip_en)?(0x01):(0x00);

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, 
        lp5810_chip_en_arr, sizeof(lp5810_chip_en_arr));
}

void lp5810_sw_reset(void)
{
    uint8_t tx_buffer[] = {0x23, 0x66};

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, 
        tx_buffer, sizeof(tx_buffer));
}

void lp5810_start_command(void)
{
    uint8_t tx_buffer[] = {0x11, 0xFF};

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, 
        tx_buffer, sizeof(tx_buffer));
}

void lp5810_update_command(void)
{
    uint8_t tx_buffer[] = {0x10, 0x55};

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, 
        tx_buffer, sizeof(tx_buffer));
}

#if LP5810_AUTO_MODE
void lp5810_send_auto_control(void)
{
    //0th byte is register address, rest 3 is data from that address
    uint8_t tx_buffer[4];
    uint32_t len;

#if 0
    for(uint32_t k=0;k<(sizeof(lp5810_auto_control)); k++)
    {
        if(0 != lp5810_auto_control[k])
        {
            tfp_printf("----");
        }
        tfp_printf("%2x %2x\r\n",k+LP5810_AUTO_CONTROL_START_ADRS,lp5810_auto_control[k]);
    }
    tfp_printf("Auto\r\n");
#endif

    for(uint32_t i=0;i<(sizeof(lp5810_auto_control)); )
    {
        if(0 == lp5810_auto_control[i])
        {
            i++;
            continue;
        }// Else is everything below

        /* How much to look ahead can be computed in case
           there is a non-zero value close to the end, now
           it is a hard coded value of 6 since LED3 isn't used.
           6 because one of the 7 non-zero data is already found. */
        len = (sizeof(tx_buffer) - 2); 
        
        while(0 == lp5810_auto_control[i+len])
        {
            len--;
        }

        tx_buffer[0] = i + LP5810_AUTO_CONTROL_START_ADRS;
        memcpy((tx_buffer+1),(lp5810_auto_control+i),len+1);
        
        i = i + len + 1;

        i2c_write(LP5810D_I2C_TARGET_ADDRESS, tx_buffer, len+2);
#if 0
        tfp_printf("L%d ", len+2);
        for(uint8_t j = 0; j < (len + 2);j++)
        {
            tfp_printf("%2x ", tx_buffer[j]);
        }
        tfp_printf("\r\n");
#endif
    }
}
#else
void lp5810_set_manual_bgr_pwm(uint8_t blue, uint8_t green, uint8_t red)
{
    uint8_t tx_buffer[4];
    tx_buffer[0] = 0x40;
    tx_buffer[1] = blue;
    tx_buffer[2] = green;
    tx_buffer[3] = red;

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, 
        tx_buffer, sizeof(tx_buffer));
}
#endif

void lp5810_init_sw_workaround(void)
{
    uint8_t tx_buffer[2];

    tx_buffer[0] = 0x00; tx_buffer[1] = 0x01;   //Broadcast Write Data 0x01 to Register 0x000
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS, tx_buffer, sizeof(tx_buffer));

    tx_buffer[0] = 0x50; tx_buffer[1] = 0x05;   //Broadcast Write Data 0x05 to Register 0x350
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS_0x3xx, tx_buffer, sizeof(tx_buffer));

    tx_buffer[0] = 0x50; tx_buffer[1] = 0x08;   //Broadcast Write Data 0x08 to Register 0x350
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS_0x3xx, tx_buffer, sizeof(tx_buffer));

    tx_buffer[0] = 0x50; tx_buffer[1] = 0x01;   //Broadcast Write Data 0x01 to Register 0x350
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS_0x3xx, tx_buffer, sizeof(tx_buffer));

    tx_buffer[0] = 0x50; tx_buffer[1] = 0x03;   //Broadcast Write Data 0x03 to Register 0x350
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS_0x3xx, tx_buffer, sizeof(tx_buffer));

    tx_buffer[0] = 0x51; tx_buffer[1] = 0x27;   //Broadcast Write Data 0x27 to Register 0x351
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS_0x3xx, tx_buffer, sizeof(tx_buffer));

    tx_buffer[0] = 0x50; tx_buffer[1] = 0x00;   //Broadcast Write Data 0x00 to Register 0x350
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS_0x3xx, tx_buffer, sizeof(tx_buffer));

    tx_buffer[0] = 0x00; tx_buffer[1] = 0x00;   //Broadcast Write Data 0x00 to Register 0x000
    i2c_write(LP5810_I2C_BROADCAST_ADDRESS, tx_buffer, sizeof(tx_buffer));
}

void lp5810_start_sequence(void)
{
    //Turn on the LP5810 IC
    lp5810_set_chip_en(true);

    lp5810_sw_reset();

    lp5810_init_sw_workaround();

    //Turn on the LP5810 IC
    lp5810_set_chip_en(true);

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, (uint8_t *)
            lp5810_dev_config_3to5, sizeof(lp5810_dev_config_3to5));

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, (uint8_t *)
            lp5810_dev_config_12, sizeof(lp5810_dev_config_12));

    lp5810_update_command();

    i2c_write(LP5810D_I2C_TARGET_ADDRESS, (uint8_t *)
            lp5810_led_en1, sizeof(lp5810_led_en1));

#if LP5810_AUTO_MODE
    i2c_write(LP5810D_I2C_TARGET_ADDRESS, (uint8_t *)
            lp5810_manual_PWM, sizeof(lp5810_manual_PWM));
    i2c_write(LP5810D_I2C_TARGET_ADDRESS, (uint8_t *)
            lp5810_auto_dc, sizeof(lp5810_auto_dc));
    
    lp5810_send_auto_control();

    lp5810_update_command();

    lp5810_start_command();
#else
    i2c_write(LP5810D_I2C_TARGET_ADDRESS, (uint8_t *)
            lp5810_manual_dc, sizeof(lp5810_manual_dc));
#endif

#if 0
    uint8_t buffer[4];

    i2c_read(LP5810D_I2C_TARGET_ADDRESS, 0x50, buffer, sizeof(buffer));

    tfp_printf("50 reg %x %x %x %x\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
#endif
}

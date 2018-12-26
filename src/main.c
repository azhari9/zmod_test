/*******************************************************************************
 * Copyright (c) 2018 Integrated Device Technology, Inc.
 * All Rights Reserved.
 *
 * This code is proprietary to IDT, and is license pursuant to the terms and
 * conditions that may be accessed at:
 * https://www.idt.com/document/msc/idt-software-license-terms-gas-sensor-software
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   This is an example for the ZMOD4410 gas sensor module.
 * @version 1.0.2
 * @date    2018-05-15
 * @author  fnaepelt
 **/

#include "main.h"

/* forward declaration */
void print_hicom_error(hicom_status_t status);

int main()
{
    /* These are the hardware handles which needs to be adjusted to specific HW */
    hicom_handle_t hicom_handle;
    hicom_status_t hicom_status;

    zmod44xx_dev_t dev;
    int8_t ret;
    uint8_t zmod44xx_status;
    float r_mox;

    /* To work with the algorithms target specific libraries needs to be
     * downloaded from IDT webpage and included into the project. */
    uint8_t iaq;
    float eco2;
    float r_cda;
    float tvoc;
//    control_signal_state_t cs_state;

    eco2_params eco2_par = {
            .min_co2 = 400,
            .max_co2 = 5000,
            .tvoc_to_eco2 = 800,
            .hot_wine_rate = 0.3,
            .open_window_rate = -0.05,
    };

    tvoc_params tvoc_par = {
            .A = 680034,
            .alpha = 0.7,
    };
/*
    odor_params odor_par = {
            .alpha = 0.8,
            .stop_delay = 24,
            .threshold = 1.3,
            .tau = 720,
            .stabilization_samples = 15,
    };
*/
    /* ****** BEGIN TARGET SPECIFIC INITIALIZATION ************************** */
    /* This part automatically resets the sensor. */
    /* connect to HiCom board */
    hicom_status = hicom_open(&hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        print_hicom_error(hicom_status);
        return hicom_status;
    }

    /* switch supply on */
    hicom_status = hicom_power_on(hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        print_hicom_error(hicom_status);
        return hicom_status;
    }

    set_hicom_handle(&hicom_handle);

    /* ****** END TARGET SPECIFIC INITIALIZATION **************************** */

    /* Set initial hardware parameter */
    dev.read = hicom_i2c_read;
    dev.write = hicom_i2c_write;
    dev.delay_ms = hicom_sleep;
    dev.i2c_addr = ZMOD4410_I2C_ADDRESS;

    /* initialize and start sensor */
    ret = zmod44xx_read_sensor_info(&dev);
    if(ret) {
        printf("Error %d, exiting program!\n", ret);
        goto exit;
    }

    ret = zmod44xx_init_sensor(&dev);
    if(ret) {
        printf("Error %d, exiting program!\n", ret);
        goto exit;
    }

    ret = zmod44xx_init_measurement(&dev);
    if(ret) {
        printf("Error %d, exiting program!\n", ret);
        goto exit;
    }

    ret = zmod44xx_start_measurement(&dev);
    if(ret) {
        printf("Error %d, exiting program!\n", ret);
        goto exit;
    }
    /* Wait for initialization finished. */
    do {
        dev.delay_ms(50);
        ret = zmod44xx_read_status(&dev, &zmod44xx_status);
        if(ret) {
            printf("Error %d, exiting program!\n", ret);
            goto exit;
        }
    } while (FIRST_SEQ_STEP != (zmod44xx_status & 0x07));

    printf("Evaluate measurements in a loop. Press any key to quit.\n");
    do {
        /* INSTEAD OF POLLING THE INTERRUPT CAN BE USED FOR OTHER HW */
        /* wait until readout result is possible */
        while (LAST_SEQ_STEP != (zmod44xx_status & 0x07)) {
            dev.delay_ms(50);
            ret = zmod44xx_read_status(&dev, &zmod44xx_status);
            if(ret) {
                printf("Error %d, exiting program!\n", ret);
                goto exit;
            }
        }

        /* evaluate and show measurement results */
        ret = zmod44xx_read_rmox(&dev, &r_mox);
        if(ret) {
            printf("Error %d, exiting program!\n", ret);
            goto exit;
        }

        /* To work with the algorithms target specific libraries needs to be
         * downloaded from IDT webpage and included into the project */

        /* calculate clean dry air resistor */
        r_cda = r_cda_tracker(r_mox);

        /* calculate result to TVOC value */
        tvoc = calc_tvoc(r_mox, r_cda, &tvoc_par);

        /* calculate IAQ index */
        iaq = calc_iaq(r_mox, r_cda, &tvoc_par);

        /* calculate estimated CO2 */
        eco2 = calc_eco2(tvoc, 3, &eco2_par);

        /* get odor control signal */
  //      cs_state = calc_odor(r_mox, &odor_par);

        printf("\n");
        printf("Measurement:\n");
        printf("  Rmox  = %5.0f kOhm\n", (r_mox / 1000.0));
        printf("  IAQ %d\n", iaq);
        printf("  TVOC %f mg/m^3\n", tvoc);
        printf("  eCO2 %f\n", eco2);
    //    printf("  odor control state %d\n", cs_state);

        /* INSTEAD OF POLLING THE INTERRUPT CAN BE USED FOR OTHER HW */
        /* waiting for sensor ready */
        while (FIRST_SEQ_STEP != (zmod44xx_status & 0x07)) {
            dev.delay_ms(50);
            ret = zmod44xx_read_status(&dev, &zmod44xx_status);
            if(ret) {
                printf("Error %d, exiting program!\n", ret);
                goto exit;
            }
        }
    } while (1);

exit:
    /* ****** BEGIN TARGET SPECIFIC DEINITIALIZATION ****** */
    hicom_status = hicom_power_off(hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        print_hicom_error(hicom_status);
        return hicom_status;
    }

    /* disconnect HiCom board */
    hicom_status = hicom_close(hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        print_hicom_error(hicom_status);
        return hicom_status;
    }
    /* ****** END TARGET SPECIFIC DEINITIALIZATION ****** */

    printf("Bye!\n");
    return 0;
}

void print_hicom_error(hicom_status_t status)
{
    char error_str[512];
    hicom_get_error_string(status, error_str, sizeof error_str);
    fprintf(stderr, "ERROR (HiCom): %s\n", error_str);
}

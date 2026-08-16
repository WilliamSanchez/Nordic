/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup blinky_example_main main.c
 * @{
 * @ingroup blinky_example
 * @brief Blinky Example Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "app_button.h"
#include "app_timer.h"
#include "bsp.h"

#define PIN_IN BUTTON_1
#define LED_3          NRF_GPIO_PIN_MAP(1,6) 
#define BUTTON_2       NRF_GPIO_PIN_MAP(1,13)

static void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
            // Disable button interrupt
    //    nrf_drv_gpiote_in_event_disable(PIN_PIN);

        // Start debounce timer
    //app_timer_start(m_debounce_timer, APP_TIMER_TICKS(30), NULL);
    nrf_gpio_pin_toggle(LED_1);

    if(pin == PIN_IN)
    {
        nrf_gpio_pin_write(LED_3,0);
    }else if(pin == BUTTON_2){
        nrf_gpio_pin_write(LED_3,1);
    }
}

static void buttons_init(void)
{
   ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_IN, &in_config, button_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_IN, true);


    /////

    nrf_drv_gpiote_in_config_t in_config2 = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config2.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(BUTTON_2, &in_config2, button_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(BUTTON_2, true);

}

int main(void)
{
    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_cfg_output(LED_2);
    nrf_gpio_cfg_output(LED_3);

    buttons_init();

    nrf_gpio_pin_write(LED_1,1);
    nrf_gpio_pin_write(LED_3,1);

    while (1)
    {
        //nrf_gpio_pin_toggle(LED_1);
        nrf_gpio_pin_toggle(LED_2);
        nrf_delay_ms(100);
    }
}

/*



#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "app_timer.h"
#include "app_error.h"

#define BUTTON_PIN          11
#define DEBOUNCE_TIME_MS    30

APP_TIMER_DEF(m_debounce_timer);


static void debounce_timeout_handler(void *p_context)
{
    // Button uses pull-up:
    // 0 = pressed
    // 1 = released

    if (nrf_gpio_pin_read(BUTTON_PIN) == 0)
    {
        // Confirmed button press
        // Put your code here
    }

    // Enable GPIOTE again
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN, true);
}


static void button_handler(nrf_drv_gpiote_pin_t pin,
                           nrf_gpiote_polarity_t action)
{
    if (pin == BUTTON_PIN &&
        action == NRF_GPIOTE_POLARITY_HITOLO)
    {
        // Disable further button events while debouncing
        nrf_drv_gpiote_in_event_disable(BUTTON_PIN);

        // Start debounce timer
        ret_code_t err_code = app_timer_start(
            m_debounce_timer,
            APP_TIMER_TICKS(DEBOUNCE_TIME_MS),
            NULL
        );

        APP_ERROR_CHECK(err_code);
    }
}


static void button_init(void)
{
    ret_code_t err_code;

    // Initialize GPIOTE
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Configure button
    nrf_drv_gpiote_in_config_t config =
        GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

    config.pull = NRF_GPIO_PIN_PULLUP;

    // Initialize GPIO
    err_code = nrf_drv_gpiote_in_init(
        BUTTON_PIN,
        &config,
        button_handler
    );

    APP_ERROR_CHECK(err_code);

    // Enable GPIOTE event
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN, true);
}


int main(void)
{
    ret_code_t err_code;

    // Initialize app_timer
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create debounce timer
    err_code = app_timer_create(
        &m_debounce_timer,
        APP_TIMER_MODE_SINGLE_SHOT,
        debounce_timeout_handler
    );

    APP_ERROR_CHECK(err_code);

    // Initialize button
    button_init();

    while (true)
    {
        __WFE();
    }
}




















/////////////////////////////////

NRF_GPIOTE_POLARITY_LOTOHI
NRF_GPIOTE_POLARITY_HITOLO
NRF_GPIOTE_POLARITY_TOGGLE

////////////////////////////////////


nrf_drv_gpiote_in_config_t button_config =
    GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);

button_config.pull = NRF_GPIO_PIN_PULLUP;

////////////////////////////////////////

static void button_handler(nrf_drv_gpiote_pin_t pin,
                           nrf_gpiote_polarity_t action)
{
    if (pin == BUTTON_PIN)
    {
        if (action == NRF_GPIOTE_POLARITY_HITOLO)
        {
            // Button pressed
        }
        else if (action == NRF_GPIOTE_POLARITY_LOTOHI)
        {
            // Button released
        }
    }
}

////////////////

{
    if (pin == BUTTON_PIN)
    {
        if (nrf_gpio_pin_read(BUTTON_PIN) == 0)
        {
            // Button pressed
        }
        else
        {
            // Button released
        }
    }
}

*/
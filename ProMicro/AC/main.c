#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "app_error.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "boards.h"

#include <stdio.h>
#include <string.h>
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_pwr_mgmt.h"


#define SAADC_CHANNEL_0    5
#define SAADC_CHANNEL_1    7

static nrf_saadc_value_t m_buffer[2];

static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(0);
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;

/**
 * SAADC callback
 *
 * With two channels, the buffer contains:
 *
 * m_buffer[0] = Channel 0 result
 * m_buffer[1] = Channel 1 result
 */
static void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        nrf_saadc_value_t ch0_value;
        nrf_saadc_value_t ch1_value;

        ch0_value = p_event->data.done.p_buffer[0];
        ch1_value = p_event->data.done.p_buffer[1];

        NRF_LOG_INFO("CH0 = %d, CH1 = %d", ch0_value, ch1_value);

        /*
         * Re-use the buffer for the next conversion.
         */
        ret_code_t err_code =
            nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, 2);

        APP_ERROR_CHECK(err_code);
        nrf_gpio_pin_toggle(LED_2);
    }
}

static void saadc_init(void)
{
    ret_code_t err_code;

    /*
     * SAADC configuration
     */
    nrf_drv_saadc_config_t saadc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;

    saadc_config.resolution = NRF_SAADC_RESOLUTION_8BIT;
    saadc_config.oversample = NRF_SAADC_OVERSAMPLE_DISABLED;
    saadc_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;

    err_code = nrf_drv_saadc_init(&saadc_config, saadc_callback);
    APP_ERROR_CHECK(err_code);


    /*
     * Channel 0 -> AIN0 / P0.02
     */
    nrf_saadc_channel_config_t channel_0_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN5);

    channel_0_config.gain = NRF_SAADC_GAIN1_6;
    channel_0_config.reference = NRF_SAADC_REFERENCE_INTERNAL;

    err_code = nrf_drv_saadc_channel_init(SAADC_CHANNEL_0, &channel_0_config);
    APP_ERROR_CHECK(err_code);


    /*
     * Channel 1 -> AIN1 / P0.03
     */
    nrf_saadc_channel_config_t channel_1_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN7);

    channel_1_config.gain = NRF_SAADC_GAIN1_6;
    channel_1_config.reference = NRF_SAADC_REFERENCE_INTERNAL;

    err_code = nrf_drv_saadc_channel_init(SAADC_CHANNEL_1, &channel_1_config);
    APP_ERROR_CHECK(err_code);


    /*
     * Give SAADC a buffer for TWO samples.
     *
     * Since we have two channels:
     *
     * buffer[0] -> CH0
     * buffer[1] -> CH1
     */
    err_code = nrf_drv_saadc_buffer_convert(m_buffer, 2);
    APP_ERROR_CHECK(err_code);
}

void timer_handler(nrf_timer_event_t event_type, void * p_context)
{

}

void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 400ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 10);
    nrf_drv_timer_extended_compare(&m_timer, NRF_TIMER_CC_CHANNEL0, ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer, NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel, timer_compare_event_addr, saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}

void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    ret_code_t err_code;

    nrf_gpio_cfg_output(LED_2);

    /*
     * Initialize logging
     */
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    /*
     * Initialize SAADC
     */
    saadc_init();

    ret_code_t ret_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(ret_code);

    saadc_sampling_event_init();
    saadc_sampling_event_enable();

    NRF_LOG_INFO("Two channel SAADC started");


    while (true)
    {
        /*
         * Start one SAADC conversion.
         *
         * Both configured channels are sampled.
         */
        //err_code = nrf_drv_saadc_sample();
        //APP_ERROR_CHECK(err_code);
        nrf_pwr_mgmt_run();
        NRF_LOG_FLUSH();
        /*
         * Allow the SAADC callback to execute.
         */
        //NRF_LOG_PROCESS();

        //nrf_delay_ms(100);
    }
}
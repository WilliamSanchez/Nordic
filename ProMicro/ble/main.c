#include <stdint.h>
#include <string.h>

#include "nrf.h"
#include "app_error.h"
#include "app_timer.h"

#include "ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "ble_nus.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

#define DEVICE_NAME                     "BLE_UART"
#define APP_BLE_CONN_CFG_TAG            1
#define APP_BLE_OBSERVER_PRIO           3

BLE_NUS_DEF(m_nus);
NRF_BLE_GATT_DEF(m_gatt);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;

static void nus_data_handler(ble_nus_evt_t * p_evt)
{
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        // Echo received data
        uint16_t len = p_evt->params.rx_data.length;

        ble_nus_data_send(&m_nus,
                          p_evt->params.rx_data.p_data,
                          &len,
                          m_conn_handle);
    }
}

static void services_init(void)
{
    ble_nus_init_t nus_init = {0};

    nus_init.data_handler = nus_data_handler;

    APP_ERROR_CHECK(ble_nus_init(&m_nus, &nus_init));
}

static void gap_params_init(void)
{
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    APP_ERROR_CHECK(sd_ble_gap_device_name_set(
        &sec_mode,
        (const uint8_t *)DEVICE_NAME,
        strlen(DEVICE_NAME)));
}

static void gatt_init(void)
{
    APP_ERROR_CHECK(nrf_ble_gatt_init(&m_gatt, NULL));
}

static void advertising_init(void)
{
    ble_gap_adv_params_t adv_params = {0};

    adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.interval = MSEC_TO_UNITS(100, UNIT_0_625_MS);
    adv_params.duration = 0;

    ble_advdata_t advdata = {0};

    advdata.name_type = BLE_ADVDATA_FULL_NAME;
    advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    ble_uuid_t adv_uuids[] =
    {
        {BLE_UUID_NUS_SERVICE, m_nus.uuid_type}
    };

    ble_advdata_t srdata = {0};

    srdata.uuids_complete.uuid_cnt = 1;
    srdata.uuids_complete.p_uuids = adv_uuids;

    APP_ERROR_CHECK(ble_advdata_set(&advdata, &srdata));

    APP_ERROR_CHECK(sd_ble_gap_adv_start(&adv_params,
                                         APP_BLE_CONN_CFG_TAG));
}

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            advertising_init();
            break;

        default:
            break;
    }
}

NRF_SDH_BLE_OBSERVER(m_ble_observer,
                     APP_BLE_OBSERVER_PRIO,
                     ble_evt_handler,
                     NULL);

static void ble_stack_init(void)
{
    APP_ERROR_CHECK(nrf_sdh_enable_request());

    uint32_t ram_start = 0;

    APP_ERROR_CHECK(nrf_sdh_ble_default_cfg_set(
        APP_BLE_CONN_CFG_TAG,
        &ram_start));

    APP_ERROR_CHECK(nrf_sdh_ble_enable(&ram_start));
}

int main(void)
{
    APP_ERROR_CHECK(app_timer_init());

    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();

    advertising_init();

    while (true)
    {
        if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
        {
            static uint32_t cnt = 0;

            char msg[32];

            sprintf(msg, "Count %lu\r\n", cnt++);

            uint16_t len = strlen(msg);

            ble_nus_data_send(&m_nus,
                              (uint8_t *)msg,
                              &len,
                              m_conn_handle);
        }

        nrf_delay_ms(1000);
    }
}
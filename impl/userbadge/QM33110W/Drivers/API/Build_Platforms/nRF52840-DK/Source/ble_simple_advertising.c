/**
 * @file ble_simple_advertising.c
 * @brief Simple BLE advertising module for QM33120WDK1
 * 
 * This module provides a simple interface to start BLE advertising
 * with a custom device name.
 */

#include "ble_simple_advertising.h"
#include <string.h>
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advdata.h"
#include "ble_gap.h"
#include "app_error.h"
#include "app_util.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define APP_BLE_CONN_CFG_TAG            1
#define APP_BLE_OBSERVER_PRIO           3
#define APP_ADV_INTERVAL                64  // 40ms in units of 0.625ms
#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static bool m_ble_enabled = false;

/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data =
{
    .adv_data =
    {
        .p_data = m_enc_advdata,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    },
    .scan_rsp_data =
    {
        .p_data = m_enc_scan_response_data,
        .len    = BLE_GAP_ADV_SET_DATA_SIZE_MAX
    }
};

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            printf("BLE: Connected\n");
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            printf("BLE: Disconnected, restarting advertising\n");
            ble_simple_advertising_start();
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing the BLE stack.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;
    bool softdevice_actually_enabled = false;

    // Check if SoftDevice is actually enabled
    if (nrf_sdh_is_enabled())
    {
        printf("BLE: SoftDevice is enabled\n");
        softdevice_actually_enabled = true;
    }
    else
    {
        printf("BLE: SoftDevice not enabled, attempting to enable...\n");
        err_code = nrf_sdh_enable_request();
        if (err_code == NRF_ERROR_INVALID_STATE)
        {
            // INVALID_STATE means SoftDevice thinks it's enabled, but nrf_sdh_is_enabled() says no
            // This is an inconsistent state - try to reset by disabling first
            printf("BLE: Inconsistent state detected, attempting to reset...\n");
            nrf_sdh_disable_request();
            nrf_delay_ms(50);
            err_code = nrf_sdh_enable_request();
            if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)
            {
                printf("BLE: ERROR - nrf_sdh_enable_request failed after reset: 0x%X\n", err_code);
                APP_ERROR_CHECK(err_code);
            }
        }
        else if (err_code != NRF_SUCCESS)
        {
            printf("BLE: ERROR - nrf_sdh_enable_request failed: 0x%X\n", err_code);
            APP_ERROR_CHECK(err_code);
        }
        
        // Wait for SoftDevice to initialize
        nrf_delay_ms(200);
        
        // Verify it's actually enabled now
        if (nrf_sdh_is_enabled())
        {
            printf("BLE: SoftDevice enabled successfully\n");
            softdevice_actually_enabled = true;
        }
        else
        {
            // Try to verify SoftDevice is actually present by calling a SoftDevice function
            ble_gap_addr_t addr;
            err_code = sd_ble_gap_addr_get(&addr);
            if (err_code == NRF_ERROR_SOFTDEVICE_NOT_ENABLED)
            {
                printf("BLE: ERROR - SoftDevice binary not loaded!\n");
                printf("BLE: You need to flash the SoftDevice hex file first:\n");
                printf("BLE: sdk/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex\n");
                printf("BLE: Or use 'Debug -> Go' which should load it automatically.\n");
            }
            else
            {
                printf("BLE: ERROR - SoftDevice in inconsistent state (err: 0x%X)\n", err_code);
            }
            return;
        }
    }
    
    // Final verification that SoftDevice is enabled
    if (!nrf_sdh_is_enabled())
    {
        printf("BLE: ERROR - SoftDevice verification failed!\n");
        printf("BLE: SoftDevice binary may not be loaded. Try:\n");
        printf("BLE: 1. Erase the device\n");
        printf("BLE: 2. Flash SoftDevice hex: sdk/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex\n");
        printf("BLE: 3. Then flash your application\n");
        return;
    }

    printf("BLE: Configuring BLE stack...\n");
    // Configure the BLE stack using the default settings.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    if (err_code != NRF_SUCCESS)
    {
        printf("BLE: ERROR - nrf_sdh_ble_default_cfg_set failed: 0x%X\n", err_code);
        APP_ERROR_CHECK(err_code);
    }

    printf("BLE: Enabling BLE stack (RAM start: 0x%X)...\n", ram_start);
    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    if (err_code == NRF_ERROR_INVALID_STATE)
    {
        // BLE stack is already enabled, get the actual RAM start
        printf("BLE: BLE stack already enabled, getting RAM start...\n");
        err_code = nrf_sdh_ble_app_ram_start_get(&ram_start);
        if (err_code == NRF_SUCCESS)
        {
            printf("BLE: RAM start is 0x%X\n", ram_start);
        }
    }
    else if (err_code != NRF_SUCCESS)
    {
        printf("BLE: ERROR - nrf_sdh_ble_enable failed: 0x%X\n", err_code);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        printf("BLE: BLE stack enabled successfully, RAM start: 0x%X\n", ram_start);
    }

    // Verify SoftDevice is still enabled
    if (!nrf_sdh_is_enabled())
    {
        printf("BLE: ERROR - SoftDevice not enabled after BLE initialization!\n");
        printf("BLE: SoftDevice binary may not be loaded. Check debug_additional_load_file in project.\n");
        return;
    }
    
    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
    
    m_ble_enabled = true;
    printf("BLE: Stack initialized successfully (RAM start: 0x%X)\n", ram_start);
}

/**@brief Function for initializing GAP parameters.
 */
static void gap_params_init(const char *device_name)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)device_name,
                                          strlen(device_name));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS);
    gap_conn_params.slave_latency     = 0;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(4000, UNIT_10_MS);

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing advertising.
 */
static void advertising_init(const char *device_name)
{
    ret_code_t    err_code;
    ble_advdata_t advdata;

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    if (err_code != NRF_SUCCESS)
    {
        printf("BLE: ERROR - ble_advdata_encode failed: 0x%X\n", err_code);
        APP_ERROR_CHECK(err_code);
    }

    // No scan response data
    m_adv_data.scan_rsp_data.len = 0;

    ble_gap_adv_params_t adv_params;

    // Set advertising parameters.
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;
    adv_params.duration        = APP_ADV_DURATION;
    adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.p_peer_addr     = NULL;
    adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    adv_params.interval        = APP_ADV_INTERVAL;

    printf("BLE: Configuring advertising (adv data len: %d)...\n", m_adv_data.adv_data.len);
    
    // Stop any existing advertising first
    if (m_adv_handle != BLE_GAP_ADV_SET_HANDLE_NOT_SET)
    {
        printf("BLE: Stopping existing advertising...\n");
        sd_ble_gap_adv_stop(m_adv_handle);
        m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    }
    
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params);
    if (err_code == NRF_ERROR_NO_MEM)
    {
        printf("BLE: ERROR - NO_MEM: SoftDevice RAM issue.\n");
        printf("BLE: Adv data len: %d bytes, GATT table size: %d bytes\n", 
               m_adv_data.adv_data.len, NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE);
        printf("BLE: Try reducing GATT table size or check if SoftDevice is properly initialized.\n");
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code != NRF_SUCCESS)
    {
        printf("BLE: ERROR - sd_ble_gap_adv_set_configure failed: 0x%X\n", err_code);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        printf("BLE: Advertising configured successfully (handle: %d)\n", m_adv_handle);
    }
}

/**@brief Function for starting advertising.
 */
void ble_simple_advertising_start(void)
{
    if (!m_ble_enabled)
    {
        printf("BLE: Stack not initialized, call ble_simple_advertising_init first\n");
        return;
    }

    printf("BLE: Starting advertising...\n");
    ret_code_t err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    if (err_code != NRF_SUCCESS)
    {
        printf("BLE: ERROR - sd_ble_gap_adv_start failed: 0x%X\n", err_code);
        APP_ERROR_CHECK(err_code);
    }
    
    printf("BLE: Advertising started successfully!\n");
}

/**@brief Function for initializing BLE advertising.
 *
 * @param[in] device_name  Name to advertise (max 31 characters)
 */
void ble_simple_advertising_init(const char *device_name)
{
    if (device_name == NULL || strlen(device_name) == 0)
    {
        device_name = "QM33120WDK1";
    }

    printf("BLE: Initializing advertising with name: %s\n", device_name);

    ble_stack_init();
    gap_params_init(device_name);
    advertising_init(device_name);
    
    printf("BLE: Advertising initialized\n");
}

/**@brief Function for processing BLE events (polling mode).
 */
void ble_simple_advertising_process(void)
{
#if (NRF_SDH_DISPATCH_MODEL == NRF_SDH_DISPATCH_MODEL_POLLING)
    nrf_sdh_evts_poll();
#endif
}


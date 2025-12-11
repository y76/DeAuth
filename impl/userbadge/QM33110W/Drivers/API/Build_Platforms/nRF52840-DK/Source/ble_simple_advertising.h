/**
 * @file ble_simple_advertising.h
 * @brief Simple BLE advertising module header for QM33120WDK1
 */

#ifndef BLE_SIMPLE_ADVERTISING_H__
#define BLE_SIMPLE_ADVERTISING_H__

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Initialize BLE advertising with a device name.
 *
 * @param[in] device_name  Name to advertise (max 31 characters). 
 *                        If NULL or empty, defaults to "QM33120WDK1"
 */
void ble_simple_advertising_init(const char *device_name);

/**@brief Start BLE advertising.
 *
 * Call this after ble_simple_advertising_init() to start advertising.
 * If a connection is lost, advertising will automatically restart.
 */
void ble_simple_advertising_start(void);

/**@brief Process BLE events (for polling mode).
 *
 * Call this periodically in your main loop if using polling dispatch model.
 * For interrupt mode, this is not needed.
 */
void ble_simple_advertising_process(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_SIMPLE_ADVERTISING_H__


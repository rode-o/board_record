#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize NVS, Wi-Fi, and connect to the given SSID/password in station mode.
 *
 * This function will block until the station gets an IP address or fails to connect
 * (it attempts a few times).
 *
 * @param ssid      Wi-Fi SSID
 * @param password  Wi-Fi password
 */
void wifi_init_sta(const char* ssid, const char* password);

#ifdef __cplusplus
}
#endif

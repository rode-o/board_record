#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Gets the station MAC address as a string, e.g. "24:6F:28:AB:CD:EF".
 * 
 * @param[out] mac_str   Buffer to store the result. Must be at least 18 chars.
 */
void wifi_utils_get_sta_mac_str(char* mac_str);

#ifdef __cplusplus
}
#endif

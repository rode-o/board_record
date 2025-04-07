#include "wifi_utils.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <cstdio>    // for sprintf

static const char* TAG = "wifi_utils";

extern "C" void wifi_utils_get_sta_mac_str(char* mac_str)
{
    // mac_str must have space for "XX:XX:XX:XX:XX:XX", i.e. 17 + null = 18
    uint8_t mac[6];
    esp_err_t err = esp_wifi_get_mac(WIFI_IF_STA, mac);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get STA MAC, error=0x%x", err);
        // fill with an error or fallback string
        sprintf(mac_str, "00:00:00:00:00:00");
        return;
    }

    sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "Station MAC: %s", mac_str);
}

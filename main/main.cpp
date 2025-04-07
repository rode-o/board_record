#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
    #include "esp_wifi.h"
    #include "esp_event.h"
    #include "nvs_flash.h"
    #include "esp_netif.h"
    #include "esp_log.h"
}

// Minimal event handler (just logs for now)
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI("wifi", "Disconnected, retrying...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        auto *event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("wifi", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

extern "C" void app_main()
{
    // Init NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Init netif & default event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create default station
    esp_netif_create_default_wifi_sta();

    // Wi-Fi init
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                ESP_EVENT_ANY_ID,
                &wifi_event_handler,
                nullptr,
                nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                IP_EVENT_STA_GOT_IP,
                &wifi_event_handler,
                nullptr,
                nullptr));

    // Set mode STA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Configure open network
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, "MyOpenNetwork", sizeof(wifi_config.sta.ssid) - 1);
    wifi_config.sta.password[0] = '\0'; // or just do .password = ""
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;  // <== key line!

    // Optionally disable PMF if needed:
    wifi_config.sta.pmf_cfg.capable  = false;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());

    // That triggers WIFI_EVENT_STA_START => esp_wifi_connect()

    // Wait forever
    while(true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

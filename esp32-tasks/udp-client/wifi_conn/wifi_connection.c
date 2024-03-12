#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <string.h>
// Includes wifi credentials
#include "wifi_config.h"
#include "wifi_connection.h"

#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#define DEFAULT_RSSI -127
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN

static const char *TAG = "WIFI";

// 192.168.1.72
char * wifi_ip_addr = NULL;
// 192.168.1.255 [0-254]
char * wifi_ip_broadcast = NULL;

void wifi_ip_address_clear() {
    if (wifi_ip_addr != NULL) {
        free(wifi_ip_addr);
        wifi_ip_addr= NULL;
    }

    if (wifi_ip_broadcast != NULL) {
        free(wifi_ip_broadcast);
        wifi_ip_broadcast = NULL;
    }
}

/**
 * parse IP address to char* and create a broadcast IP address for current subnet
*/
void wifi_ip_address_save(esp_ip4_addr_t ip) {
    int ipSize = 16;
    wifi_ip_addr = (char *) malloc(ipSize);
    wifi_ip_broadcast = (char *) malloc(ipSize);
    snprintf(wifi_ip_addr, ipSize, IPSTR, IP2STR(&ip));
    snprintf(wifi_ip_broadcast, ipSize, IPSTR, esp_ip4_addr1_16(&ip), esp_ip4_addr2_16(&ip), esp_ip4_addr3_16(&ip), 255);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGI(TAG, "Received event: %s %" PRId32 , event_base, event_id);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        wifi_ip_address_clear();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        wifi_ip_address_clear();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_ip_address_clear();
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        wifi_ip_address_save(event->ip_info.ip);
        ESP_LOGI(TAG, "got ip: %s broadcast: %s", wifi_ip_addr, wifi_ip_broadcast);
    }
}

/* Initialize Wi-Fi as sta and set scan method */
void wifi_connect() {
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_init());
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    // Initialize default station as network interface instance (esp-netif)
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // Initialize and start WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PW,
            .scan_method = DEFAULT_SCAN_METHOD,
            .sort_method = DEFAULT_SORT_METHOD,
            .threshold = {
                .rssi = DEFAULT_RSSI,
                .authmode = DEFAULT_AUTHMODE,
            },
        },
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());
}
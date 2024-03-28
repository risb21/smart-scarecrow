#include <esp_log.h>
#include "esp_system.h"
#include "esp_event.h"
#include <esp_system.h>
#include <nvs_flash.h>

#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "../wifi_conn/wifi_connection.h"
#include "../wifi_conn/wifi_connection.c"

#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "esp_camera.h"

// Uncomment only if using the ESP32-CAM
#define BOARD_ESP32CAM_AITHINKER

#ifdef BOARD_ESP32CAM_AITHINKER

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10

#endif

#if defined(CONFIG_EXAMPLE_IPV4)
    #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#else
    #define HOST_IP_ADDR ""
#endif

//static const char *TAG = "UDP-Client";

//static EventGroupHandle_t wifi_event_group;

// retry tracker
//static int s_retry_num = 0;

// task tag
//static const char *TAG = "WIFI";

#if ESP_CAMERA_SUPPORTED
static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 6, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t init_camera(void) {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    } 
    return ESP_OK;
}
#endif

// static void udp_client_task(void *pvParameters) {
//     char rx_buffer[128];
//     char host_ip[] = HOST_IP_ADDR;
//     int addr_family = 0;
//     int ip_protocol = 0;

//     while (true) {
//         #if defined(CONFIG_EXAMPLE_IPV4)
//             struct sockaddr_in dest_addr;
//             dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
//             dest_addr.sin_family = AF_INET;
//             dest_addr.sin_port = htons(PORT);
//             addr_family = AF_INET;
//             ip_protocol = IPPROTO_IP;
//         #elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
//             struct sockaddr_storage dest_addr = { 0 };
//             ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
//         #endif

//         int udp_sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
//         if (udp_sock < 0) {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

//         while (1) {

//             int err = sendto(udp_sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//             if (err < 0) {
//                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                 break;
//             }
//             ESP_LOGI(TAG, "Message sent");

//             struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
//             socklen_t socklen = sizeof(source_addr);
//             int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

//             // Error occurred during receiving
//             if (len < 0) {
//                 ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
//                 break;
//             }
//             // Data received
//             else {
//                 rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
//                 ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
//                 ESP_LOGI(TAG, "%s", rx_buffer);
//                 if (strncmp(rx_buffer, "OK: ", 4) == 0) {
//                     ESP_LOGI(TAG, "Received expected message, reconnecting");
//                     break;
//                 }
//             }

//             vTaskDelay(2000 / portTICK_PERIOD_MS);
//         }

//         if (udp_sock != -1) {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(udp_sock, 0);
//             close(udp_sock);
//         }
//     }
//     vTaskDelete(NULL);
// }


// void app_main(void) {

// #ifndef ESP_CAMERA_SUPPORTED
//     ESP_LOGE(TAG, "Code Written for AI Thinker ESP32 CAM, cannot run on another platform.");
//     return;
// #else

//     if (init_camera() != ESP_OK) {
//         return;
//     }
    
//     // Initialize storage
//     esp_err_t retval = nvs_flash_init();
//     if (retval == ESP_ERR_NVS_NO_FREE_PAGES || retval == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_erase());
//         retval = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK_WITHOUT_ABORT(retval);
    
//     // Connect to wifi
//     wifi_connect();

//     while (true) {
//         ESP_LOGI(TAG, "Taking a picture...");
        
//         camera_fb_t *img_frame_buffer = esp_camera_fb_get();
//         ESP_LOGI(TAG, "Image taken, %zu bytes", img_frame_buffer -> len);
        
//         esp_camera_fb_return(img_frame_buffer);
//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }

// #endif
// }

void app_main(void)
{
	esp_err_t status = WIFI_FAILURE;

    #ifndef ESP_CAMERA_SUPPORTED
        ESP_LOGE(TAG, "Code Written for AI Thinker ESP32 CAM, cannot run on another platform.");
        return;
    #else
        if (init_camera() != ESP_OK) {
            return;
        }

	//initialize storage
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

    // connect to wireless AP
	    status = connect_wifi();
	    if (WIFI_SUCCESS != status)
	    {
		    ESP_LOGI(TAG, "Failed to associate to AP, dying...");
		    return;
	    }
	
	// status = connect_tcp_server();
	// if (TCP_SUCCESS != status)
	// {
	// 	ESP_LOGI(TAG, "Failed to connect to remote server, dying...");
	// 	return;
	// }

    #endif
}

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
#include "../UDP_sockets/udp_socket.h"

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

static const char *TAG = "UDP-Client";

static esp_err_t init_camera(void) {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    } 
    return ESP_OK;
}
#endif


void app_main(void){
	esp_err_t status = WIFI_FAILURE;

    #ifndef ESP_CAMERA_SUPPORTED
        ESP_LOGE(TAG, "Code Written for AI Thinker ESP32 CAM, cannot run on another platform.");
        return;
    #else

        if (init_camera() != ESP_OK) {
            ESP_LOGE(TAG, "Could not initialize camera, dying...");
            goto death;
        }

	    //initialize storage
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

        // connect to wireless AP
	    status = connect_wifi();
	    if (WIFI_SUCCESS != status){
		    ESP_LOGE(TAG, "Failed to associate to AP, dying...");
            goto death;
	    }

        if (wifi_ip_addr != NULL) {
            ESP_LOGI(TAG, "Connected to the internet! IP Address: %s", wifi_ip_addr);
        }

        camera_data_t camera_data;
        camera_data.has_data = false;
        camera_data.in_use_mtx = xSemaphoreCreateMutex();
        camera_data.cam_frame_buf = NULL;

        xTaskCreate(udp_client_task, "udp-client", 4096, (void *) &camera_data, 5, NULL);

        
        while (true) {
            // Block for 10 ms, continue with camera capture
            if (xSemaphoreTake(camera_data.in_use_mtx, (TickType_t) (10 / portTICK_PERIOD_MS)) != pdTRUE) {
                // Wait and retry if mutex is not free
                vTaskDelay((TickType_t) (50 / portTICK_PERIOD_MS));
                continue;
            }
            
            ESP_LOGI(TAG, "Taking a picture...");

            camera_data.cam_frame_buf = esp_camera_fb_get();
            ESP_LOGI(TAG, "Image taken, %zu bytes", camera_data.cam_frame_buf -> len);
            ESP_LOGI(TAG, "%dX%d pixels", camera_data.cam_frame_buf -> width, camera_data.cam_frame_buf -> height);

            esp_camera_fb_return(camera_data.cam_frame_buf);

            xSemaphoreGive(camera_data.in_use_mtx);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
        
        return;

        death:
        {
            for (;;) {
                vTaskDelay(5000 / portTICK_PERIOD_MS);
            }
        }
    #endif
}
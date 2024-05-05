#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <string.h>
#include <sys/param.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "esp_camera.h"

typedef struct {
    SemaphoreHandle_t in_use_mtx;
    camera_fb_t *cam_frame_buf;
} camera_data_t;

void udp_client_task(void *param_args);

#endif
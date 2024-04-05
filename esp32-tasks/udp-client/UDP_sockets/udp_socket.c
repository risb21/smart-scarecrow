#include "udp_socket.h"
#include "server_config.h"

static const char *TAG = "UDP Socket Connection";
static const char *payload = "Is this working?";
camera_data_t *camera_data;
// SemaphoreHandle_t camera_data -> in_use_mtx;

void udp_client_task(void *param_args) {

    char rx_buffer[128];
    char server_ip[] = SERVER_IP;
    int addr_family = 0;
    int ip_protocol = 0; // UDP
    struct sockaddr_in server_addr;
    camera_data = (camera_data_t *) param_args;

    while (1) {
        struct sockaddr_in *server = &server_addr;
        
        u32_t server_ip_valid = inet_addr(SERVER_IP);
        if (server_ip_valid == INADDR_NONE) {
            ESP_LOGE(TAG, "Invalid server IP Address, Dying...");
            goto death;
        }
        
        server -> sin_addr.s_addr = server_ip_valid;
        server -> sin_family = AF_INET;
        server -> sin_port = htons(SERVER_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int client_sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (client_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket");
            goto death;
        }

        // Setting timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        while (1) {
            // Block for 10 ms, continue with transfer if free
            if (xSemaphoreTake(camera_data -> in_use_mtx, (TickType_t) (10 / portTICK_PERIOD_MS)) == pdFALSE) {
                // Wait and retry if mutex is not free
                vTaskDelay((TickType_t) (50 / portTICK_PERIOD_MS));
                continue;
            }

            int err = sendto(
                client_sock, payload, strlen(payload), 0,
                (struct sockaddr *) &server_addr, sizeof(server_addr)
            );
            
            if (err < 0) {
                ESP_LOGE(TAG, "Could not send message to server");
                xSemaphoreGive(camera_data -> in_use_mtx);
                break;
            } 
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_storage source_addr;
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(
                client_sock, rx_buffer, sizeof(rx_buffer)-1,
                0, (struct sockaddr *) &source_addr, &socklen
            );

            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed");
                xSemaphoreGive(camera_data -> in_use_mtx);
                break;
            } else {
                rx_buffer[len] = 0; // Null terminate string received
                ESP_LOGI(TAG, "Received %d bytes from host %s:", len, server_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
            }

            ESP_LOGI(TAG, "Checking param bool value: %d", camera_data -> has_data);
            xSemaphoreGive(camera_data -> in_use_mtx);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (client_sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(client_sock, 0);
            close(client_sock);
        }
    }

    vTaskDelete(NULL);
    
    death:
    {  // Loop forever after dying
        for (;;) {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

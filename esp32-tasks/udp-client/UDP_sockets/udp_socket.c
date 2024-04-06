#include "udp_socket.h"
#include "server_config.h"

static const char *TAG = "UDP Socket Connection";
static const char *payload = "Is this working?";
static const char *done_message = "DONE";
camera_data_t *camera_data;

// #define OUTER_BRACES "[%s]"
#define DEF_PIXEL    "(%d,%d,%d),"
// #define APPEND       "%s,%s"
#define SERVER_RX_LIMIT 4096

char* get_payload(camera_fb_t *cam_fb, int row_no) {
    sensor_t* sensor_data = esp_camera_sensor_get();
    if (!sensor_data) {
        return NULL;
    }

    if (sensor_data -> pixformat != PIXFORMAT_RGB565) {
        ESP_LOGE(TAG, "Other pixel formats are not implemented, returning empty string");
        return NULL;
    }

    // 8192 bytes for max rx limit server-side
    char *row = (char *) malloc(SERVER_RX_LIMIT * sizeof(char));
    row[0] = '[';
    int row_ptr = 1;

    /*
        // finding max length of string to be sent
        // () + 3x 2 digit 8-bit nos, with 2 ',' separating them.
        int max_pix_len = 2 + 3*2 + 2;
        // [] + no. of pixels in a row * nomax_pix_len + 
        // (no. of pixels in a row - 1) ',' separating them
        int max_row_len = 2 + width * max_pix_len + (width-1);
        // ESP_LOGI(TAG, "Max size of row of pixels: %d bytes", max_row_len);
    */

    int width = cam_fb -> width;
    // each pixel is 2 bytes, getting how many rows to skip in indices
    int rows_skipped = (row_no * width *2);
    for (int w = 0; w < width; w++) {
        //    Byte 1       |     Byte 2
        // x x x x x x x x | x x x x x x x x
        //  Red(5)  |   Green(6)  |  Blue(5)
        u8_t B1 = cam_fb -> buf[rows_skipped + 2*w + 0];
        u8_t B2 = cam_fb -> buf[rows_skipped + 2*w + 1];

        // Keep first 5 bits of B1
        u8_t red = B1 >> 3;
        // keep last 3 bits of B1, then put in position
        // push first 3 bits of B2 to the end & join them together
        u8_t green = (B1 & 0x07) << 3 | B2 >> 5;
        // Keep last 5 bits of B2
        u8_t blue = B2 & 0x1F;

        row_ptr += snprintf(row + row_ptr, SERVER_RX_LIMIT, DEF_PIXEL, red, green, blue);
    }
    row[row_ptr - 1] = ']';
    // Null terminate string
    row[row_ptr] = '\0';

    // ESP_LOGI(TAG, "Row string generated:");
    // ESP_LOGI(TAG, "%s", row);
    // ESP_LOGI(TAG, "Row string length: %d bytes", row_ptr);    

    // free(row);

    return row;
}

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

            camera_fb_t *cam_frame_buff = esp_camera_fb_get();

            if (!cam_frame_buff) {
                ESP_LOGE(TAG, "Could not get camera frame buffer");
                xSemaphoreGive(camera_data -> in_use_mtx);
                break;
            }

            for (int h = 0; h < cam_frame_buff -> height; h++) {
                char* row_payload = get_payload(cam_frame_buff, h);
                
                if (!row_payload) {
                    h--;
                    continue;
                }

                int err = sendto(
                    client_sock, row_payload, strlen(row_payload), 0,
                    (struct sockaddr *) &server_addr, sizeof(server_addr)
                );

                free(row_payload);

                if (err < 0) {
                    h--;
                    continue;
                }

                struct sockaddr_storage source_addr;
                socklen_t socklen = sizeof(source_addr);
                int len = recvfrom(
                    client_sock, rx_buffer, sizeof(rx_buffer)-1,
                    0, (struct sockaddr *) &source_addr, &socklen
                );

                if (len < 0) {
                    h--;
                    continue;
                }

                rx_buffer[len] = 0; // Null terminate string received
                ESP_LOGI(TAG, "Received %d bytes from host %s:", len, server_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);

                // Only resend if requested by server
                if (len == strlen("RESEND") && strcmp(rx_buffer, "RESEND") != 0) {
                    h--;
                    continue;
                }
            }

            esp_camera_fb_return(cam_frame_buff);


            int err = sendto(
                client_sock, done_message, strlen(done_message), 0,
                (struct sockaddr *) &server_addr, sizeof(server_addr)
            );

            while (err < 0) {
                err = sendto(
                    client_sock, done_message, strlen(done_message), 0,
                    (struct sockaddr *) &server_addr, sizeof(server_addr)
                );
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

            // ESP_LOGI(TAG, "Checking param bool value: %d", camera_data -> has_data);
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

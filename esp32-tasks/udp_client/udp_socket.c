#include "udp_socket.h"
#include "server_config.h"

static const char *TAG = "UDP Socket Connection";
static const char *done_message = "DONE";


// Returns length of payload, and pointer to data in *data
int get_raw_payload(camera_fb_t *cam_fb, int start_fb, uint8_t *data) {
    sensor_t* sensor_data = esp_camera_sensor_get();
    if (!sensor_data) {
        return -1;
    }

    // if (sensor_data -> pixformat != PIXFORMAT_RGB565 && sensor_data -> pixformat != PIXFORMAT_JPEG) {
    //     ESP_LOGE(TAG, "Other pixel formats are not implemented");
    //     return -1;
    // }

    int data_len = cam_fb -> len;

    for (int i = start_fb;
         i < ((data_len > start_fb + SERVER_BUFF_SIZE) ?
              start_fb + SERVER_BUFF_SIZE  :  data_len); i++) {
        data[i - start_fb] = cam_fb -> buf[i];
    }

    return (data_len > start_fb + SERVER_BUFF_SIZE) ? SERVER_BUFF_SIZE : data_len - start_fb;
}

void udp_client_task(void *param_args) {

    char rx_buffer[128];
    char server_ip[] = SERVER_IP;
    int addr_family = 0;
    int ip_protocol = 0; // UDP
    struct sockaddr_in server_addr;

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
        timeout.tv_sec = 0;
        timeout.tv_usec = 7500;
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        int end_delim = 0;
        char dimensions_str[40];

        while (1) {
            camera_fb_t *cam_frame_buff = esp_camera_fb_get();

            if (!cam_frame_buff) {
                ESP_LOGE(TAG, "Could not get camera frame buffer");
                break;
            }

            int fb_len = cam_frame_buff -> len;

            // Preliminary packet to indicate image size and dimensions
            end_delim = snprintf(
                dimensions_str, 40, "%zux%zu %zu",
                cam_frame_buff -> width,
                cam_frame_buff -> height,
                fb_len
            );

            dimensions_str[end_delim] = '\0';

            int send_err = sendto(
                client_sock, dimensions_str, strlen(dimensions_str), 0,
                (struct sockaddr *) &server_addr, sizeof(server_addr)
            );

            if (send_err < 0) {
                esp_camera_fb_return(cam_frame_buff);
                // ESP_LOGE(TAG, "Unable to send dimension/image size data, retyring...");
                continue;
            }
            
            struct sockaddr_storage src_addr_dim;
            socklen_t dim_socklen = sizeof(src_addr_dim);
            int recv_len = recvfrom(
                client_sock, rx_buffer, sizeof(rx_buffer)-1,
                0, (struct sockaddr *) &src_addr_dim, &dim_socklen
            );

            rx_buffer[recv_len] = '\0';

            int chunks = (int) (fb_len / SERVER_BUFF_SIZE);
            uint8_t *raw_payload = (uint8_t *) malloc((SERVER_BUFF_SIZE) * sizeof(uint8_t));

            for (int i = 0;
                 i < chunks + (
                    (fb_len % SERVER_BUFF_SIZE) > 0 ? 1 : 0
                 ); i++) {
                
                int len = get_raw_payload(cam_frame_buff, i * SERVER_BUFF_SIZE, raw_payload);

                if (len < 0) {
                    i--;
                    ESP_LOGE(TAG, "Unable to get raw payload");
                    continue;
                }

                int err = sendto(
                    client_sock, raw_payload, len, 0,
                    (struct sockaddr *) &server_addr, sizeof(server_addr)
                );

                if (err < 0) {
                    i--;
                    continue;
                }
            }

            free(raw_payload);
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

            // ESP_LOGI(TAG, "Image sent");
        }

        if (client_sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(client_sock, 0);
            close(client_sock);
        }
    }

    death:
        vTaskDelete(NULL);
}

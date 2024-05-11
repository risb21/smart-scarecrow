#include "tcp_servo_control.h"

#define PORT                        4200
#define KEEPALIVE_IDLE              5
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             3

static const char *TAG = "TCP Servo Server";

static void do_retransmit(const int sock) {
    int len;
    char rx_buffer[128];
    char num1[128];
    char num2[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            int idx = 0, old;
            while (rx_buffer[idx] != ',') {
                num1[idx] = rx_buffer[idx];
                idx++;
            }
            num1[idx++] = '\0';
            old = idx;
            while (rx_buffer[idx] != '\0') {
                num2[idx - old] = rx_buffer[idx];
                idx++;
            }
            num2[idx] = '\0';

            move_servo_360(atoi(num1));
            move_servo_180(atoi(num2));
            vTaskDelay(50 / portTICK_PERIOD_MS);

            gpio_set_level(GPIO_NUM_0, 1);
            ESP_LOGI(TAG, "Set pin 0 to 1");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_0, 0);
            ESP_LOGI(TAG, "Set pin 0 to 0");
            vTaskDelay(5000 / portTICK_PERIOD_MS);

        }
    } while (len > 0);
}

void tcp_server_task(void *pvParameters) {
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    servo_param_360 servos = {
        .pin1 = 13,
        .pin2 = 12,
    };

    servo_param_360 servo = {
        .pin1 = 15,
    };

    setup_servo_360(&servos);
    setup_servo_180(&servo);

    // gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    // gpio_set_level(GPIO_NUM_0, 0);

        // while (true) {
        //     gpio_set_level(GPIO_NUM_0, 1);
        //     ESP_LOGE(TAG, "Set pin 0 to 1");
        //     vTaskDelay(2000 / portTICK_PERIOD_MS);
        //     gpio_set_level(GPIO_NUM_0, 0);
        //     ESP_LOGE(TAG, "Set pin 0 to 0");
        //     vTaskDelay(2000 / portTICK_PERIOD_MS);
        // }

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *) &dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}
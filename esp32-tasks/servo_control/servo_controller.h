#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include "math.h"

typedef struct {
    int pin1;
    int pin2;
} servo_param_360;

void servo_handler_task(void *param_args);

void setup_servo_180(servo_param_360 *servos);
void move_servo_180(float angle);

void setup_servo_360(servo_param_360 *servos);
void move_servo_360(float angle);

#endif
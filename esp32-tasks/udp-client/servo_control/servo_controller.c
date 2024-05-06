#include "servo_controller.h"

#define SERVO_MIN_PULSEWIDTH_US 500
#define SERVO_MAX_PULSEWIDTH_US 2500
#define SERVO_MIN_ANGLE        -90
#define SERVO_MAX_ANGLE        90

// #define SERVO_PULSE_GPIO             0        // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD        20000    // 20000 ticks, 20ms

static const char *TAG = "Servo Control";

static inline uint32_t servo_angle_to_duty_us(int angle) {
    return (angle + SERVO_MAX_ANGLE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_ANGLE) + SERVO_MIN_PULSEWIDTH_US;
}

void servo_handler_task(void *param_args) {

    int gpio_pin = (int) param_args;
    bool forwards = true;
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, gpio_pin); // To drive a RC servo, one MCPWM generator is enough

    mcpwm_config_t pwm_config = {
        .frequency = 1000000 / SERVO_TIMEBASE_PERIOD , // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,                                   // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    while (1) {
        for (int angle = SERVO_MIN_ANGLE; angle < SERVO_MAX_ANGLE; angle++) {
            // ESP_LOGI(TAG, "Angle of rotation: %d", angle);
            ESP_ERROR_CHECK_WITHOUT_ABORT(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, servo_angle_to_duty_us(angle * (forwards ? 1 : -1))));
            //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation under 5V power supply
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        forwards = !forwards;
    }

    vTaskDelete(NULL);
}
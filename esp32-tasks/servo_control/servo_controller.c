#include "servo_controller.h"

#define SERVO_MIN_PULSEWIDTH_US 625
#define SERVO_MAX_PULSEWIDTH_US 2375
#define SERVO_MIN_ANGLE        -90
#define SERVO_MAX_ANGLE        90

// #define SERVO_PULSE_GPIO             0        // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD        20000    // 20000 ticks, 20ms

static const char *TAG = "Servo Control";

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define round(a) (((a % 1) >= 0.5) ? ((a - (a % 1)) + 1) : (a - (a % 1)))

static inline uint32_t servo_angle_to_duty_us(float angle) {
    return (angle + SERVO_MAX_ANGLE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_ANGLE) + SERVO_MIN_PULSEWIDTH_US;
}

static inline float duty_us_to_servo_angle(uint32_t duty) {
    return /*round*/(((float) (duty - SERVO_MIN_PULSEWIDTH_US)) * (2 * SERVO_MAX_ANGLE) / (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) - SERVO_MAX_ANGLE);
} 

static inline void sweep_180(mcpwm_unit_t unit, mcpwm_timer_t timer, mcpwm_generator_t gen, float start, float end,  bool forwards) {
    for (float angle = start; angle < end; angle += 0.5) {
        // ESP_LOGI(TAG, "%f", angle * (forwards ? 1 : -1));
        ESP_ERROR_CHECK_WITHOUT_ABORT(
            mcpwm_set_duty_in_us(
                unit, timer, gen, 
                servo_angle_to_duty_us(angle * (forwards ? 1 : -1))
            )
        );
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void servo_handler_task(void *param_args) {

    servo_param_360 *servos = (servo_param_360 *) param_args;
    bool forwards = true;
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, servos -> pin1);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, servos -> pin2);

    mcpwm_config_t pwm_config = {
        .frequency = 1000000 / SERVO_TIMEBASE_PERIOD , // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,                                   // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);

    while (1) {
        sweep_180(MCPWM_UNIT_0, (forwards ? MCPWM_TIMER_0 : MCPWM_TIMER_1), MCPWM_GEN_A, -90, 90, forwards);
        sweep_180(MCPWM_UNIT_0, (forwards ? MCPWM_TIMER_1 : MCPWM_TIMER_0), MCPWM_GEN_A, -90, 90, forwards);
        forwards = !forwards;
    }

    vTaskDelete(NULL);
}

// ALWAYS use before moving servos
void setup_servo_360(servo_param_360 *servos) {
    // bool forwards = true;
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, servos -> pin1);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, servos -> pin2);
    mcpwm_config_t pwm_config = {
        .frequency = 1000000 / SERVO_TIMEBASE_PERIOD , // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        // .cmpr_a = 0,                                   // duty cycle of PWMxB = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);
    // ESP_LOGI(TAG, "Duty for -90: %lu", (unsigned long) servo_angle_to_duty_us(-90));
    mcpwm_set_duty_in_us( MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_GEN_A, servo_angle_to_duty_us(-90));
    mcpwm_set_duty_in_us( MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_GEN_A, servo_angle_to_duty_us(-90));
}

// ALWAYS use before moving servos
void setup_servo_180(servo_param_360 *servos) {
    // bool forwards = true;
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM2B, servos -> pin1);
    mcpwm_config_t pwm_config = {
        .frequency = 1000000 / SERVO_TIMEBASE_PERIOD , // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        // .cmpr_b = 0,                                   // duty cycle of PWMxB = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_2, &pwm_config);
    // ESP_LOGI(TAG, "Duty for -90: %lu", (unsigned long) servo_angle_to_duty_us(-90));
    mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_2, MCPWM_GEN_B, servo_angle_to_duty_us(-90));
}

void move_servo_360(float angle) {
    // float duty1 = mcpwm_get_duty(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM0A)/100;
    // float duty2 = mcpwm_get_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM0A)/100;
    // float angle1 = duty_us_to_servo_angle((uint32_t) (duty1 * SERVO_TIMEBASE_PERIOD));
    // float angle2 = duty_us_to_servo_angle((uint32_t) (duty2 * SERVO_TIMEBASE_PERIOD));

    // bool forwards = (angle1 + angle2 + 180) <= angle ? true : false;

    // ESP_LOGI(TAG, "angle test: %.3f", duty_us_to_servo_angle(servo_angle_to_duty_us(angle > 180 ? angle - 270 : angle - 90)));

    mcpwm_set_duty_in_us(
        MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_GEN_A,
        servo_angle_to_duty_us(min(angle - 90, 90))
    );
    vTaskDelay(10 / portTICK_PERIOD_MS);
    mcpwm_set_duty_in_us(
        MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_GEN_A,
        servo_angle_to_duty_us(max(angle - 270, -90))
    );
}

void move_servo_180(float angle) {
    // float duty1 = mcpwm_get_duty(MCPWM_UNIT_1, MCPWM_TIMER_2, MCPWM2B)/100;
    // float angle1 = duty_us_to_servo_angle((uint32_t) (duty1 * SERVO_TIMEBASE_PERIOD));

    // bool forwards = (angle1 + angle2 + 180) <= angle ? true : false;

    // ESP_LOGI(TAG, "angle test: %.3f", duty_us_to_servo_angle(servo_angle_to_duty_us(angle > 180 ? angle - 270 : angle - 90)));

    mcpwm_set_duty_in_us(
        MCPWM_UNIT_1, MCPWM_TIMER_2, MCPWM_GEN_B,
        servo_angle_to_duty_us(angle - 90)
    );
}
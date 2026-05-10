/**
 * Servo Module - Implementation
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "servo.h"

static const char *TAG = "SERVO";

#define SERVO_FREQ_HZ     50
#define SERVO_MIN_US      500
#define SERVO_MAX_US      2400
#define LEDC_TIMER_RES    LEDC_TIMER_14_BIT
#define LEDC_PERIOD_TICKS (1 << 14)

// GPIO assignments for ESP32-S3-DevKitC-1 v1.1
static const int servo_pins[NUM_SERVOS] = {
    4,   // S0: Front Left Hip
    5,   // S1: Front Left Knee
    6,   // S2: Rear Left Hip
    7,   // S3: Rear Left Knee
    10,  // S4: Front Right Hip
    11,  // S5: Front Right Knee
    12,  // S6: Rear Right Hip
    13,  // S7: Rear Right Knee
};

static const char *servo_names[NUM_SERVOS] = {
    "FL Hip", "FL Knee",
    "RL Hip", "RL Knee",
    "FR Hip", "FR Knee",
    "RR Hip", "RR Knee",
};

static int servo_angles[NUM_SERVOS] = {90, 90, 90, 90, 90, 90, 90, 90};

static uint32_t angle_to_duty(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    uint32_t pulse_us = SERVO_MIN_US
        + (uint32_t)((SERVO_MAX_US - SERVO_MIN_US) * angle / 180);
    uint32_t duty = (uint32_t)((uint64_t)pulse_us * LEDC_PERIOD_TICKS / 20000);
    return duty;
}

void servo_init(void)
{
    ledc_timer_config_t timer0 = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_RES,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer0));

    ledc_timer_config_t timer1 = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_RES,
        .timer_num       = LEDC_TIMER_1,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer1));

    for (int i = 0; i < NUM_SERVOS; i++) {
        ledc_channel_config_t ch = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel    = (ledc_channel_t)i,
            .timer_sel  = (i < 4) ? LEDC_TIMER_0 : LEDC_TIMER_1,
            .intr_type  = LEDC_INTR_DISABLE,
            .gpio_num   = servo_pins[i],
            .duty       = angle_to_duty(90),
            .hpoint     = 0,
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ch));
    }

    ESP_LOGI(TAG, "All 8 servo channels initialized");
}

void servo_set_angle(int servo_id, int angle)
{
    if (servo_id < 0 || servo_id >= NUM_SERVOS) {
        ESP_LOGW(TAG, "Invalid servo ID: %d", servo_id);
        return;
    }
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    uint32_t duty = angle_to_duty(angle);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)servo_id, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)servo_id);

    servo_angles[servo_id] = angle;
    ESP_LOGD(TAG, "S%d (%s) -> %d°", servo_id, servo_names[servo_id], angle);
}

int servo_get_angle(int servo_id)
{
    if (servo_id < 0 || servo_id >= NUM_SERVOS) return -1;
    return servo_angles[servo_id];
}

void servo_center_all(void)
{
    ESP_LOGI(TAG, "Centering all servos (staggered)...");
    for (int i = 0; i < NUM_SERVOS; i++) {
        servo_set_angle(i, 90);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

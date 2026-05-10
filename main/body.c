/**
 * Body Tilt & Height - Implementation
 */

#include <math.h>
#include "body.h"
#include "ik.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"

static const char *TAG = "BODY";

// Body dimensions (mm) - measure on your robot
#define BODY_HALF_LENGTH  50.0f   // Center of body to hip (front-back), adjust to your frame
#define BODY_HALF_WIDTH   40.0f   // Center of body to hip (left-right), adjust to your frame

// Current state
static float pitch = 0.0f;
static float roll = 0.0f;
static float height = L2;

static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

void body_set_pitch(float pitch_deg)
{
    pitch = clampf(pitch_deg, -15.0f, 15.0f);
    ESP_LOGI(TAG, "Pitch: %.1f°", pitch);
}

void body_set_roll(float roll_deg)
{
    roll = clampf(roll_deg, -15.0f, 15.0f);
    ESP_LOGI(TAG, "Roll: %.1f°", roll);
}

void body_set_height(float height_mm)
{
    height = clampf(height_mm, 20.0f, L2);
    ESP_LOGI(TAG, "Height: %.1f mm", height);
}

float body_get_pitch(void) { return pitch; }
float body_get_roll(void) { return roll; }
float body_get_height(void) { return height; }

// ---------------------------------------------------------------------------
// Apply body tilt and height to all legs
// ---------------------------------------------------------------------------

void body_apply(void)
{

    float pitch_rad = pitch * M_PI / 180.0f;
    float dz_pitch = BODY_HALF_LENGTH * sinf(pitch_rad);

    float roll_rad = roll * M_PI / 180.0f;
    float dz_roll = BODY_HALF_WIDTH * sinf(roll_rad);

    float FL_z = height + dz_pitch + dz_roll;
    float FR_z = height + dz_pitch - dz_roll;
    float RL_z = height - dz_pitch + dz_roll;
    float RR_z = height - dz_pitch - dz_roll;

    foot_pos_t fl_pos = { .x = 0, .z = FL_z };
    ik_move_leg(LEG_FL, fl_pos);
    vTaskDelay(pdMS_TO_TICKS(20));

    foot_pos_t fr_pos = { .x = 0, .z = FR_z };
    ik_move_leg(LEG_FR, fr_pos);
    vTaskDelay(pdMS_TO_TICKS(20));

    foot_pos_t rl_pos = { .x = 0, .z = RL_z };
    ik_move_leg(LEG_RL, rl_pos);
    vTaskDelay(pdMS_TO_TICKS(20));

    foot_pos_t rr_pos = { .x = 0, .z = RR_z };
    ik_move_leg(LEG_RR, rr_pos);
    vTaskDelay(pdMS_TO_TICKS(20));

    ESP_LOGI(TAG, "Body applied: pitch=%.1f roll=%.1f height=%.1f",
             pitch, roll, height);
}

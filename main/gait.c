/**
 * Gait Engine
 */

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gait.h"
#include "ik.h"
#include "esp_log.h"

static const char *TAG = "GAIT";

// Gait parameters
#define MAX_STRIDE_LENGTH  20.0f   // Max forward step distance (mm)
#define LIFT_HEIGHT         12.0f  // How high foot lifts during swing (mm)
#define GAIT_TICK_MS        20     // Tick interval (ms)

// State
static gait_mode_t current_mode = GAIT_IDLE;
static float current_speed = 0.0f;
static float gait_phase = 0.0f;  // 0.0 to 1.0, wraps around

// Leg phase offsets for crawl gait (each leg is 0. 25 apart)
static const float leg_phase_offset[NUM_LEGS] = {
    [LEG_FL] = 0.00f,
    [LEG_RR] = 0.25f,
    [LEG_FR] = 0.50f,
    [LEG_RL] = 0.75f,
};

void gait_init(void)
{
    current_mode = GAIT_IDLE;
    current_speed = 0.0f;
    gait_phase = 0.0f;
    ESP_LOGI(TAG, "Gait engine initialized");
}

void gait_set_mode(gait_mode_t mode, float speed)
{
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;

    if (mode != current_mode) {
        gait_phase = 0.0f;  // Reset phase on mode change
    }

    current_mode = mode;
    current_speed = speed;
    ESP_LOGI(TAG, "Gait: mode=%d speed=%.2f", mode, speed);
}

gait_mode_t gait_get_mode(void)
{
    return current_mode;
}

// ---------------------------------------------------------------------------
// Compute foot position for one leg at current gait phase
// ---------------------------------------------------------------------------

static foot_pos_t compute_leg_position(leg_id_t leg)
{
    foot_pos_t pos = { .x = 0.0f, .z = L2 };  // Default: standing

    if (current_mode == GAIT_IDLE) {
        return pos;
    }

    float leg_phase = fmodf(gait_phase + leg_phase_offset[leg], 1.0f);

    float stride = MAX_STRIDE_LENGTH * current_speed;

    if (leg_phase < 0.25) {

    float swing_t = leg_phase / 0.25;
    pos.x = stride * sinf(M_PI * swing_t);
    pos.z =  L2 - LIFT_HEIGHT * sinf(M_PI * swing_t);
    }

    else {

    float stance_t = (leg_phase - 0.25) / 0.75;
    pos.x = stride * (1.0 - 2.0 * stance_t);
    pos.z = L2;
    }

    if (current_mode == GAIT_WALK_BACKWARD){
        pos.x = -pos.x;
    }
    
    return pos;
}

// ---------------------------------------------------------------------------
// Gait tick - advance phase and move all legs
// ---------------------------------------------------------------------------

void gait_tick(void)
{
    if (current_mode == GAIT_IDLE) return;

    float phase_increment = current_speed * 0.02;
    gait_phase += phase_increment;
    if (gait_phase >= 1.0f){
        
        gait_phase -= 1.0f;
    }
    foot_pos_t pos_FL = compute_leg_position(LEG_FL);
    foot_pos_t pos_FR = compute_leg_position(LEG_FR);
    foot_pos_t pos_RL = compute_leg_position(LEG_RL);
    foot_pos_t pos_RR = compute_leg_position(LEG_RR);

    ik_move_leg(LEG_FL, pos_FL);
    ik_move_leg(LEG_FR, pos_FR);
    ik_move_leg(LEG_RL, pos_RL);
    ik_move_leg(LEG_RR, pos_RR);

}

void gait_stop(void)
{
    current_mode = GAIT_IDLE;
    current_speed = 0.0f;
    gait_phase = 0.0f;
    ik_stand();
    ESP_LOGI(TAG, "Gait stopped, standing");
}

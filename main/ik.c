/**
 * Inverse Kinematics - Implementation
 * Fill in each TODO function.
 */

#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ik.h"
#include "servo.h"
#include "esp_log.h"

static const char *TAG = "IK";

// --- FILL IN YOUR CALIBRATION OFFSETS ---
static const leg_offsets_t leg_offsets[NUM_LEGS] = {
    [LEG_FL] = { .hip_offset = 5, .knee_offset = 5 },  // S0, S1
    [LEG_RL] = { .hip_offset = 2, .knee_offset = 5 },  // S2, S3
    [LEG_FR] = { .hip_offset = -6, .knee_offset = 3 },  // S4, S5
    [LEG_RR] = { .hip_offset = 0, .knee_offset = -3 },  // S6, S7
};

static const int leg_servo_ids[NUM_LEGS][2] = {
    [LEG_FL] = { 0, 1 },
    [LEG_RL] = { 2, 3 },
    [LEG_FR] = { 4, 5 },
    [LEG_RR] = { 6, 7 },
};

static inline float deg_to_rad(float deg) { return deg * (float)M_PI / 180.0f; }
static inline float rad_to_deg(float rad) { return rad * 180.0f / (float)M_PI; }
static inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// ---------------------------------------------------------------------------
// Forward Kinematics
// ---------------------------------------------------------------------------
foot_pos_t ik_forward(ik_angles_t angles)
{
    foot_pos_t pos;
    pos.x = L1*sinf(deg_to_rad(angles.theta1)); 
    pos.z = L2*sinf(deg_to_rad(angles.theta2)); 
    return pos;
}

// ---------------------------------------------------------------------------
// Inverse Kinematics
// ---------------------------------------------------------------------------
bool ik_inverse(foot_pos_t target, ik_angles_t *out)
{
    bool reachable = true;

    float x = clampf(target.x, -L1, L1);
    float z = clampf(target.z, -L2, L2);
    if (x != target.x || z != target.z){
        reachable = false;
    }

    out->theta1 = rad_to_deg(asinf((x/L1)));
    out->theta2 = rad_to_deg(asinf((z/L2))); 

    return reachable;
}

// ---------------------------------------------------------------------------
// IK Angles to Servo Angles
// ---------------------------------------------------------------------------
void ik_to_servo(leg_id_t leg, ik_angles_t angles, leg_offsets_t offsets,
                 int *out_hip, int *out_knee)
{
    int hip = 90, knee = 90;

    switch (leg)
    {
    case LEG_FL:
    hip = 90 - (int)angles.theta1 + offsets.hip_offset;
    knee = 90 - (int)angles.theta2 + offsets.knee_offset;      
        break;
    case LEG_RL:
    hip = 90 - (int)angles.theta1 + offsets.hip_offset;
    knee = 90 + (int)angles.theta2 + offsets.knee_offset;      
        break;
    case LEG_FR:
    hip = 90 + (int)angles.theta1 + offsets.hip_offset;
    knee = 90 + (int)angles.theta2 + offsets.knee_offset;      
        break;        
    case LEG_RR:
    hip = 90 + (int)angles.theta1 + offsets.hip_offset;
    knee = 90 - (int)angles.theta2 + offsets.knee_offset;      
        break;
    
    default:
        break;
    }

    *out_hip = clampi(hip, 0, 180);
    *out_knee = clampi(knee, 0, 180);
}

// ---------------------------------------------------------------------------
//  Move a leg to foot position
// ---------------------------------------------------------------------------
bool ik_move_leg(leg_id_t leg, foot_pos_t target)
{
    ik_angles_t angles;
    int hip, knee;
    bool reachable = ik_inverse(target, &angles);
    ik_to_servo(leg, angles, leg_offsets[leg], &hip, &knee);
    servo_set_angle(leg_servo_ids[leg][0], hip);
    servo_set_angle(leg_servo_ids[leg][1], knee);
    return reachable; 
    ESP_LOGI(TAG, "Leg %d --> x=%.1f z=%.1f | hip =%d knee =%d| %s", leg, target.x, target.z,hip,
        knee, reachable ? "OK" : "Clamped");
    }


// ---------------------------------------------------------------------------
// Stand pose
// ---------------------------------------------------------------------------
void ik_stand(void)
{
    foot_pos_t stand = {.x = 0, .z = L2};
    ik_move_leg(LEG_FL, stand);
    vTaskDelay(pdMS_TO_TICKS(20));

    ik_move_leg(LEG_FR, stand);
    vTaskDelay(pdMS_TO_TICKS(20));

    ik_move_leg(LEG_RR, stand);
    vTaskDelay(pdMS_TO_TICKS(20));

    ik_move_leg(LEG_RL, stand);
    vTaskDelay(pdMS_TO_TICKS(20));


    ESP_LOGI(TAG, "Stand complete");
}

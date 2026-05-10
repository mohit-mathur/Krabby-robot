/**
 * Inverse Kinematics Module
 * 
 * 2-DOF decoupled IK for L-shaped leg:
 *   Hip (horizontal plane): θ1 controls x (forward/backward)
 *   Knee (vertical plane):  θ2 controls z (height)
 *
 * Coordinates: X = forward(+), Y = right(+), Z = down(+)
 */

#ifndef IK_H
#define IK_H

#include <stdbool.h>

#define L1  38.1f      // Upper leg length (mm)
#define L2  47.625f    // Lower leg length (mm)

typedef enum {
    LEG_FL = 0,   // Front Left  (S0 hip, S1 knee)
    LEG_RL = 1,   // Rear Left   (S2 hip, S3 knee)
    LEG_FR = 2,   // Front Right (S4 hip, S5 knee)
    LEG_RR = 3,   // Rear Right  (S6 hip, S7 knee)
    NUM_LEGS = 4
} leg_id_t;

typedef struct {
    float x;  // forward (+) / backward (-)
    float z;  // down (+) / up (-)
} foot_pos_t;

typedef struct {
    float theta1;  // hip: 0 = sideways, + = forward
    float theta2;  // knee: 0 = extended, 90 = standing
} ik_angles_t;

typedef struct {
    int hip_offset;
    int knee_offset;
} leg_offsets_t;


foot_pos_t ik_forward(ik_angles_t angles);
bool ik_inverse(foot_pos_t target, ik_angles_t *out);
void ik_to_servo(leg_id_t leg, ik_angles_t angles, leg_offsets_t offsets,
                 int *out_hip, int *out_knee);
bool ik_move_leg(leg_id_t leg, foot_pos_t target);
void ik_stand(void);

#endif // IK_H

/**
 * Preset Poses Module
 * 
 * Each pose is a sequence of foot positions for all 4 legs.
 * Complex poses (wave, pushup, dance) are multi-frame sequences.
 */

#ifndef POSES_H
#define POSES_H

typedef enum {
    POSE_STAND = 0,
    POSE_SIT,
    POSE_LAY,
    POSE_WAVE,
    POSE_PUSHUP,
    POSE_DANCE,
} pose_id_t;

/**
 * Execute a preset pose.
 * 
 * Simple poses (stand, sit, lay): move all legs to target positions.
 * Animated poses (wave, pushup, dance): run a sequence of keyframes.
 *
 * POSE DEFINITIONS :
 *
 *   STAND:  x=0, z=L2 for all legs (you already have this in ik_stand)
 *
 *   SIT:    Front legs: x=0, z=L2 (standing)
 *           Rear legs:  x=0, z=20 (knees folded up, butt down)
 *
 *   LAY:    All legs: x=0, z=10 (barely extended, body on ground)
 *
 *   WAVE:   Multi-frame animation:
 *           Frame 1: Stand
 *           Frame 2: Shift weight to 3 legs (FR/RL/RR lower, FL raises)
 *           Frame 3-6: FL sweeps left-right a few times (change x)
 *           Frame 7: Return to stand
 *
 *   PUSHUP: Multi-frame loop:
 *           Frame 1: Stand (z = L2)
 *           Frame 2: Lower body (z = 25 for all legs)
 *           Frame 3: Raise body (z = L2)
 *           Repeat N times
 *
 *   DANCE:  Combine tilts, height changes,
 *           and individual leg movements into a choreographed sequence.
 */
void pose_execute(pose_id_t pose);

/**
 * Check if a pose animation is currently running.
 */
bool pose_is_running(void);

/**
 * Stop any running pose animation.
 */
void pose_stop(void);

#endif // POSES_H

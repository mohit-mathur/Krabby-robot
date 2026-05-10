/**
 * Gait Engine Module
 * 
 * Generates cyclic foot trajectories for walking and turning.
 * The gait engine runs in the motion control task at 50Hz (20ms loop).
 */

#ifndef GAIT_H
#define GAIT_H

typedef enum {
    GAIT_IDLE = 0,
    GAIT_WALK_FORWARD,
    GAIT_WALK_BACKWARD,
    GAIT_TURN_LEFT,
    GAIT_TURN_RIGHT,
    GAIT_CREEP,
} gait_mode_t;

/**
 * Initialize gait engine.
 */
void gait_init(void);

/**
 * Set the current gait mode and speed.
 * speed: 0.0 (stopped) to 1.0 (max speed)
 */
void gait_set_mode(gait_mode_t mode, float speed);

/**
 * Get the current gait mode.
 */
gait_mode_t gait_get_mode(void);

/**
 * Advance the gait by one tick (called every 20ms).
 *
 * This is the core function you implement. Each tick:
 *   1. Advance the gait phase (0.0 to 1.0, wrapping)
 *   2. For each leg, compute the foot position based on phase
 *   3. Call ik_move_leg() for each leg
 *
 * CRAWL GAIT (statically stable, 1 leg moves at a time):
 *   Phase 0.00 - 0.25: Lift and swing LEG_FL forward
 *   Phase 0.25 - 0.50: Lift and swing LEG_RR forward
 *   Phase 0.50 - 0.75: Lift and swing LEG_FR forward
 *   Phase 0.75 - 1.00: Lift and swing LEG_RL forward
 *   While one leg swings, the other 3 push backward (stance phase)
 *
 * MATH:
 *   Swing phase: leg lifts up (reduce z), moves forward (increase x)
 *     - x follows a half-sine: x = stride_length * sin(π * leg_phase)
 *     - z follows a half-sine: z = L2 - lift_height * sin(π * leg_phase)
 *   Stance phase: leg stays on ground, pushes backward
 *     - x moves linearly backward: x = stride_length * (1 - 2*leg_phase)
 *     - z stays at L2 (on ground)
 *
 *   stride_length: how far forward each step goes (mm), scaled by speed
 *   lift_height: how high the foot lifts (mm), e.g. 10-15mm
 */
void gait_tick(void);

/**
 * Stop all movement, return to standing.
 */
void gait_stop(void);

#endif // GAIT_H

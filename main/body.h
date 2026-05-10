/**
 * Body Tilt & Height Module
 * 
 * Shifts all 4 foot positions to tilt the body or change height.
 * Works by adjusting the IK target for each leg.
 */

#ifndef BODY_H
#define BODY_H

/**
 *Set body pitch (forward/backward tilt).
 * 
 * pitch_deg: positive = nose down, negative = nose up
 * Range: -15 to +15 degrees
 *
 * MATH:
 *   When the body tilts forward by P degrees, the front feet need to
 *   move DOWN and the rear feet need to move UP to achieve the tilt.
 *   
 *   The body half-length (hip to center) determines how much z changes:
 *     delta_z = body_half_length * sin(pitch_deg)
 *   
 *   Front legs: z = L2 + delta_z  (lower)
 *   Rear legs:  z = L2 - delta_z  (higher)
 */
void body_set_pitch(float pitch_deg);

/**
 * Set body roll (left/right tilt).
 * 
 * roll_deg: positive = lean right, negative = lean left
 * Range: -15 to +15 degrees
 *
 * Same math as pitch but applied to left/right legs.
 */
void body_set_roll(float roll_deg);

/**
 * Set body height.
 * 
 * height_mm: distance from hip to ground (mm)
 * Range: 20 to L2 (47.625)
 * Standing default: L2
 */
void body_set_height(float height_mm);

/**
 * Get current body tilt and height.
 */
float body_get_pitch(void);
float body_get_roll(void);
float body_get_height(void);

/**
 * Apply current body adjustments to all legs.
 * Called by the motion control task after gait or during idle.
 */
void body_apply(void);

#endif // BODY_H

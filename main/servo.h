/**
 * Servo Module - Low-level PWM servo control
 * 
 * Handles LEDC peripheral setup and angle-to-duty conversion
 * for all 8 MG90S servos on ESP32-S3-DevKitC-1 v1.1.
 */

#ifndef SERVO_H
#define SERVO_H

#define NUM_SERVOS 8

/**
 * Initialize all 8 servo PWM channels.
 * All servos start at 90 degrees.
 */
void servo_init(void);

/**
 * Set a single servo to a specific angle (0-180).
 * Includes bounds checking and logging.
 */
void servo_set_angle(int servo_id, int angle);

/**
 * Get the current commanded angle of a servo.
 */
int servo_get_angle(int servo_id);

/**
 * Center all servos to 90 degrees with 20ms stagger.
 */
void servo_center_all(void);

#endif // SERVO_H

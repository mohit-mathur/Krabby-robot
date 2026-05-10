/**
 * OLED Display Module
 * 
 * SSD1306 128x64 I2C OLED driver with emotion face system.
 * GPIO 8 = SDA, GPIO 9 = SCL
 */

#ifndef OLED_H
#define OLED_H

typedef enum {
    MOOD_HAPPY = 0,
    MOOD_SLEEPY,
    MOOD_ANGRY,
    MOOD_SURPRISED,
    MOOD_SAD,
    MOOD_LOVE,
    MOOD_IDLE,
    NUM_MOODS,
} mood_t;

/**
 * Initialize the SSD1306 OLED display over I2C.
 */
void oled_init(void);

/**
 * Set the current mood/emotion face.
 */
void oled_set_mood(mood_t mood);

/**
 * Get the current mood.
 */
mood_t oled_get_mood(void);

/**
 * Update the display (call periodically from display task).
 * Handles blink animation and mood transitions.
 */
void oled_update(void);

/**
 * Clear the display.
 */
void oled_clear(void);

#endif // OLED_H

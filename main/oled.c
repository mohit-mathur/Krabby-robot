/**
 * OLED Display - Implementation
 * SSD1306 128x64 I2C driver with emotion faces.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "oled.h"
#include "esp_log.h"
#include <math.h>


static const char *TAG = "OLED";

#define I2C_PORT       I2C_NUM_0
#define SDA_PIN        8
#define SCL_PIN        9
#define SSD1306_ADDR   0x3C
#define SCREEN_W       128
#define SCREEN_H       64

static uint8_t framebuf[SCREEN_W * SCREEN_H / 8];
static mood_t current_mood = MOOD_IDLE;
static int blink_counter = 0;
static bool eyes_closed = false;

// --- I2C & SSD1306 low-level ---

static esp_err_t ssd1306_cmd(uint8_t cmd)
{
    uint8_t buf[2] = { 0x00, cmd };
    return i2c_master_write_to_device(I2C_PORT, SSD1306_ADDR, buf, 2, pdMS_TO_TICKS(100));
}

static void ssd1306_init_seq(void)
{
    ssd1306_cmd(0xAE);  // Display off
    ssd1306_cmd(0xD5); ssd1306_cmd(0x80);  // Clock div
    ssd1306_cmd(0xA8); ssd1306_cmd(0x3F);  // Multiplex 64
    ssd1306_cmd(0xD3); ssd1306_cmd(0x00);  // Display offset
    ssd1306_cmd(0x40);  // Start line
    ssd1306_cmd(0x8D); ssd1306_cmd(0x14);  // Charge pump
    ssd1306_cmd(0x20); ssd1306_cmd(0x00);  // Horizontal addressing
    ssd1306_cmd(0xA1);  // Segment remap
    ssd1306_cmd(0xC0);  // COM scan dec
    ssd1306_cmd(0xDA); ssd1306_cmd(0x12);  // COM pins
    ssd1306_cmd(0x81); ssd1306_cmd(0xCF);  // Contrast
    ssd1306_cmd(0xD9); ssd1306_cmd(0xF1);  // Pre-charge
    ssd1306_cmd(0xDB); ssd1306_cmd(0x40);  // VCOMH
    ssd1306_cmd(0xA4);  // Resume RAM
    ssd1306_cmd(0xA6);  // Normal display
    ssd1306_cmd(0xAF);  // Display on
}

static void ssd1306_flush(void)
{
    ssd1306_cmd(0x21); ssd1306_cmd(0); ssd1306_cmd(127);  // Column range
    ssd1306_cmd(0x22); ssd1306_cmd(0); ssd1306_cmd(7);    // Page range

    uint8_t buf[SCREEN_W + 1];
    buf[0] = 0x40;  // Data mode
    for (int page = 0; page < 8; page++) {
        memcpy(&buf[1], &framebuf[page * SCREEN_W], SCREEN_W);
        i2c_master_write_to_device(I2C_PORT, SSD1306_ADDR, buf, SCREEN_W + 1, pdMS_TO_TICKS(100));
    }
}

// --- Drawing helpers ---

static void fb_clear(void)
{
    memset(framebuf, 0, sizeof(framebuf));
}

static void fb_pixel(int x, int y, bool on)
{
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    int page = y / 8;
    int bit = y % 8;
    if (on) {
        framebuf[page * SCREEN_W + x] |= (1 << bit);
    } else {
        framebuf[page * SCREEN_W + x] &= ~(1 << bit);
    }
}

static void fb_fill_circle(int cx, int cy, int r, bool on)
{
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r*r) {
                fb_pixel(cx + dx, cy + dy, on);
            }
        }
    }
}

static void fb_fill_rect(int x, int y, int w, int h, bool on)
{
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            fb_pixel(x + dx, y + dy, on);
        }
    }
}

static void fb_draw_arc(int cx, int cy, int r, int start_deg, int end_deg, bool on)
{
    for (int deg = start_deg; deg <= end_deg; deg++) {
        float rad = deg * 3.14159f / 180.0f;
        int x = cx + (int)(r * cosf(rad));
        int y = cy + (int)(r * sinf(rad));
        fb_pixel(x, y, on);
    }
}

// --- Face drawing ---

static void draw_eyes(int left_x, int right_x, int y, int r, bool closed)
{
    if (closed) {
        fb_fill_rect(left_x - r, y, r * 2, 2, true);
        fb_fill_rect(right_x - r, y, r * 2, 2, true);
    } else {
        fb_fill_circle(left_x, y, r, true);
        fb_fill_circle(right_x, y, r, true);
    }
}

static void draw_face_happy(bool blink)
{
    fb_clear();
    draw_eyes(40, 88, 20, 8, blink);
    // Smile: arc at bottom
    for (int x = 40; x <= 88; x++) {
        int y = 45 + (int)(8 * sinf((x - 40) * 3.14159f / 48.0f));
        fb_pixel(x, y, true);
        fb_pixel(x, y + 1, true);
    }
}

static void draw_face_sleepy(bool blink)
{
    fb_clear();
    // Half-closed eyes (always look sleepy)
    fb_fill_rect(32, 22, 16, 3, true);
    fb_fill_rect(80, 22, 16, 3, true);
    // Small "zzz"
    fb_pixel(110, 10, true); fb_pixel(111, 10, true); fb_pixel(112, 10, true);
    fb_pixel(113, 6, true); fb_pixel(114, 6, true); fb_pixel(115, 6, true);
    // Flat mouth
    fb_fill_rect(50, 48, 28, 2, true);
}

static void draw_face_angry(bool blink)
{
    fb_clear();
    draw_eyes(40, 88, 24, 7, blink);
    // Angry eyebrows (angled down toward center)
    for (int i = 0; i < 16; i++) {
        fb_pixel(30 + i, 12 + i / 3, true);
        fb_pixel(98 - i, 12 + i / 3, true);
    }
    // Frown
    for (int x = 44; x <= 84; x++) {
        int y = 52 - (int)(5 * sinf((x - 44) * 3.14159f / 40.0f));
        fb_pixel(x, y, true);
        fb_pixel(x, y + 1, true);
    }
}

static void draw_face_surprised(bool blink)
{
    fb_clear();
    // Big round eyes
    fb_fill_circle(40, 22, 10, true);
    fb_fill_circle(88, 22, 10, true);
    fb_fill_circle(40, 22, 5, false);  // Hollow center
    fb_fill_circle(88, 22, 5, false);
    // Round mouth
    fb_fill_circle(64, 50, 8, true);
    fb_fill_circle(64, 50, 5, false);
}

static void draw_face_sad(bool blink)
{
    fb_clear();
    draw_eyes(40, 88, 20, 7, blink);
    // Sad eyebrows
    for (int i = 0; i < 16; i++) {
        fb_pixel(30 + i, 10 - i / 4, true);
        fb_pixel(98 - i, 10 - i / 4, true);
    }
    // Frown (inverted smile)
    for (int x = 44; x <= 84; x++) {
        int y = 48 - (int)(6 * sinf((x - 44) * 3.14159f / 40.0f));
        fb_pixel(x, y, true);
    }
}

static void draw_face_love(bool blink)
{
    fb_clear();
    // Heart-shaped eyes (simplified)
    // Left heart
    fb_fill_circle(36, 18, 5, true);
    fb_fill_circle(44, 18, 5, true);
    for (int y = 18; y <= 30; y++) {
        int half = (30 - y) * 10 / 12;
        fb_fill_rect(40 - half, y, half * 2, 1, true);
    }
    // Right heart
    fb_fill_circle(84, 18, 5, true);
    fb_fill_circle(92, 18, 5, true);
    for (int y = 18; y <= 30; y++) {
        int half = (30 - y) * 10 / 12;
        fb_fill_rect(88 - half, y, half * 2, 1, true);
    }
    // Smile
    for (int x = 40; x <= 88; x++) {
        int y = 45 + (int)(8 * sinf((x - 40) * 3.14159f / 48.0f));
        fb_pixel(x, y, true);
    }
}

static void draw_face_idle(bool blink)
{
    fb_clear();
    draw_eyes(40, 88, 22, 6, blink);
    // Neutral mouth
    fb_fill_rect(48, 46, 32, 2, true);
}

// --- Public API ---

void oled_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    vTaskDelay(pdMS_TO_TICKS(100));
    ssd1306_init_seq();
    fb_clear();
    ssd1306_flush();

    ESP_LOGI(TAG, "OLED initialized (128x64 SSD1306)");
}

void oled_set_mood(mood_t mood)
{
    if (mood < NUM_MOODS) {
        current_mood = mood;
        ESP_LOGI(TAG, "Mood set: %d", mood);
    }
}

mood_t oled_get_mood(void) { return current_mood; }

void oled_update(void)
{
    // Blink logic: close eyes briefly every ~3 seconds
    blink_counter++;
    if (blink_counter > 30) {  // At 10 FPS, ~3 seconds
        eyes_closed = true;
        if (blink_counter > 32) {  // Eyes closed for 2 frames
            eyes_closed = false;
            blink_counter = 0;
        }
    }

    switch (current_mood) {
        case MOOD_HAPPY:    draw_face_happy(eyes_closed);    break;
        case MOOD_SLEEPY:   draw_face_sleepy(eyes_closed);   break;
        case MOOD_ANGRY:    draw_face_angry(eyes_closed);    break;
        case MOOD_SURPRISED:draw_face_surprised(eyes_closed);break;
        case MOOD_SAD:      draw_face_sad(eyes_closed);      break;
        case MOOD_LOVE:     draw_face_love(eyes_closed);     break;
        case MOOD_IDLE:     draw_face_idle(eyes_closed);     break;
        default:            draw_face_idle(eyes_closed);     break;
    }

    ssd1306_flush();
}

void oled_clear(void)
{
    fb_clear();
    ssd1306_flush();
}

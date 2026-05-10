/**
 * Quadruped Firmware - Main Entry Point
 * 
 * Creates FreeRTOS tasks for motion control, display, and networking.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "servo.h"
#include "ik.h"
#include "gait.h"
#include "body.h"
#include "poses.h"
#include "oled.h"
#include "wifi.h"
#include "webserver.h"

static const char *TAG = "MAIN";

// ---------------------------------------------------------------------------
// Motion Control Task (Core 1, 20ms loop)
// ---------------------------------------------------------------------------

static void motion_task(void *pvParams)
{
    ESP_LOGI(TAG, "Motion task started on core %d", xPortGetCoreID());

    while (1) {
        // If a gait is active, advance it one tick
        if (gait_get_mode() != GAIT_IDLE) {
            gait_tick();
        }

        vTaskDelay(pdMS_TO_TICKS(20));  // 50 Hz control loop
    }
}

// ---------------------------------------------------------------------------
// OLED Display Task (Core 0, 100ms loop = 10 FPS)
// ---------------------------------------------------------------------------

static void display_task(void *pvParams)
{
    ESP_LOGI(TAG, "Display task started on core %d", xPortGetCoreID());

    oled_init();
    oled_set_mood(MOOD_HAPPY);

    while (1) {
        oled_update();
        vTaskDelay(pdMS_TO_TICKS(100));  // 10 FPS
    }
}

// ---------------------------------------------------------------------------
// Serial Command Task (for debugging without WiFi)
// ---------------------------------------------------------------------------

static int read_line(char *buf, int max_len)
{
    int i = 0;
    while (i < max_len - 1) {
        int c = getchar();
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (c == '\n' || c == '\r') {
            buf[i] = '\0';
            putchar('\n');
            return i;
        }
        buf[i++] = (char)c;
        putchar(c);
    }
    buf[i] = '\0';
    return i;
}

static void serial_task(void *pvParams)
{
    ESP_LOGI(TAG, "Serial debug task started");
    char buf[64];

    while (1) {
        printf("> ");
        fflush(stdout);
        int len = read_line(buf, sizeof(buf));
        if (len <= 0) continue;

        // Servo commands
        if (buf[0] == 's' && buf[1] >= '0' && buf[1] <= '7') {
            int id = buf[1] - '0';
            int angle = atoi(buf + 2);
            servo_set_angle(id, angle);
        }
        else if (strcmp(buf, "center") == 0) { servo_center_all(); }
        else if (strcmp(buf, "stand") == 0) { ik_stand(); }
        else if (strcmp(buf, "status") == 0) {
            printf("Servos: ");
            for (int i = 0; i < NUM_SERVOS; i++) printf("S%d=%d ", i, servo_get_angle(i));
            printf("\nGait: %d\n", gait_get_mode());
        }
        // IK commands
        else if (strncmp(buf, "leg ", 4) == 0) {
            int leg; float x, z;
            if (sscanf(buf + 4, "%d %f %f", &leg, &x, &z) == 3) {
                foot_pos_t pos = {.x = x, .z = z};
                ik_move_leg((leg_id_t)leg, pos);
            }
        }
        else if (strncmp(buf, "fk ", 3) == 0) {
            float t1, t2;
            if (sscanf(buf + 3, "%f %f", &t1, &t2) == 2) {
                ik_angles_t a = {.theta1 = t1, .theta2 = t2};
                foot_pos_t p = ik_forward(a);
                printf("FK: x=%.2f z=%.2f\n", p.x, p.z);
            }
        }
        // Gait commands
        else if (strcmp(buf, "walk") == 0) { gait_set_mode(GAIT_WALK_FORWARD, 0.5f); }
        else if (strcmp(buf, "stop") == 0) { gait_stop(); }
        // Pose commands
        else if (strcmp(buf, "sit") == 0) { pose_execute(POSE_SIT); }
        else if (strcmp(buf, "lay") == 0) { pose_execute(POSE_LAY); }
        else if (strcmp(buf, "wave") == 0) { pose_execute(POSE_WAVE); }
        else if (strcmp(buf, "pushup") == 0) { pose_execute(POSE_PUSHUP); }
        else if (strcmp(buf, "dance") == 0) { pose_execute(POSE_DANCE); }
        // Help
        else if (strcmp(buf, "help") == 0) {
            printf("Commands: s<N> <angle>, center, stand, status\n");
            printf("  leg <N> <x> <z>, fk <t1> <t2>\n");
            printf("  walk, stop, sit, lay, wave, pushup, dance\n");
        }
        else {
            printf("Unknown: '%s'. Type 'help'\n", buf);
        }
    }
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Quadruped Robot Firmware v1.0");
    ESP_LOGI(TAG, "  ESP32-S3 | 8-DOF | WiFi Control");
    ESP_LOGI(TAG, "========================================");

    // Initialize hardware
    servo_init();
    servo_center_all();
    gait_init();

    // Initialize networking
    wifi_init_ap();
    webserver_start();

    // Create tasks
    // Motion control on Core 1 (real-time priority)
    xTaskCreatePinnedToCore(motion_task, "motion", 8192, NULL, 5, NULL, 1);

    // OLED display on Core 0
    xTaskCreatePinnedToCore(display_task, "display", 4096, NULL, 2, NULL, 0);

    // Serial debug on Core 0 (low priority)
    xTaskCreatePinnedToCore(serial_task, "serial", 4096, NULL, 1, NULL, 0);

    ESP_LOGI(TAG, "All tasks started. Connect to WiFi 'QuadrupedBot' pw 'robot1234'");
    ESP_LOGI(TAG, "Then browse to http://192.168.4.1");
}

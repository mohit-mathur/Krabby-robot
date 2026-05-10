/**
 * Preset Poses - Implementation
 */

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "poses.h"
#include "ik.h"
#include "body.h"
#include "esp_log.h"

static const char *TAG = "POSES";

static bool running = false;

// Helper: move all 4 legs with stagger delay
static void move_all_legs(foot_pos_t fl, foot_pos_t rl, foot_pos_t fr, foot_pos_t rr)
{
    ik_move_leg(LEG_FL, fl);
    vTaskDelay(pdMS_TO_TICKS(20));
    ik_move_leg(LEG_RL, rl);
    vTaskDelay(pdMS_TO_TICKS(20));
    ik_move_leg(LEG_FR, fr);
    vTaskDelay(pdMS_TO_TICKS(20));
    ik_move_leg(LEG_RR, rr);
    vTaskDelay(pdMS_TO_TICKS(20));
}

// ---------------------------------------------------------------------------
// Stand pose (provided as reference)
// ---------------------------------------------------------------------------

static void pose_stand(void)
{
    foot_pos_t stand = { .x = 0, .z = L2 };
    move_all_legs(stand, stand, stand, stand);
    ESP_LOGI(TAG, "Pose: Stand");
}

// ---------------------------------------------------------------------------
// Sit pose
// ---------------------------------------------------------------------------

static void pose_sit(void)
{
    foot_pos_t sit_front = { .x = 0, .z = L2}; //standing
    foot_pos_t sit_rear = { .x = 0, .z = 20}; //folded
    move_all_legs(sit_front, sit_rear, sit_front, sit_rear);
    ESP_LOGI(TAG, "Pose: Sit");
}

// ---------------------------------------------------------------------------
// Lay down pose
// ---------------------------------------------------------------------------

static void pose_lay(void)
{
    foot_pos_t lay = { .x = 0, .z = 10}; //standing
    move_all_legs(lay, lay, lay, lay);
    ESP_LOGI(TAG, "Pose: Lay");
}

// ---------------------------------------------------------------------------
//Wave pose (animated)
// ---------------------------------------------------------------------------

static void pose_wave(void)
{
    running = true;

    pose_stand();
    vTaskDelay(pdMS_TO_TICKS(300));

    foot_pos_t shift_weight_three = { .x = 0, .z = L2};
    foot_pos_t shift_fl = { .x = 10, .z = -20};
    move_all_legs(shift_fl, shift_weight_three, shift_weight_three, shift_weight_three);

    foot_pos_t wave_1 = { .x = 15, .z = -20};
    foot_pos_t wave_2 = { .x = -15, .z = -20};

    ik_move_leg(LEG_FL, wave_1);
    if (!running) return;
    vTaskDelay(pdMS_TO_TICKS(300));
    ik_move_leg(LEG_FL, wave_2);
    if (!running) return;
    vTaskDelay(pdMS_TO_TICKS(300));
    ik_move_leg(LEG_FL, wave_1);
    if (!running) return;
    vTaskDelay(pdMS_TO_TICKS(300));
    ik_move_leg(LEG_FL, wave_2);
    if (!running) return;
    vTaskDelay(pdMS_TO_TICKS(300));
    ik_move_leg(LEG_FL, wave_1);
    if (!running) return;
    vTaskDelay(pdMS_TO_TICKS(300));
    ik_move_leg(LEG_FL, wave_2);
    if (!running) return;
    vTaskDelay(pdMS_TO_TICKS(300));

    pose_stand();
    
    if (!running) return;

    running = false;
    ESP_LOGI(TAG, "Pose: Wave complete");
}

// ---------------------------------------------------------------------------
//Push-up pose (animated)
// ---------------------------------------------------------------------------

static void pose_pushup(void)
{
    running = true;
    int reps = 5;
    foot_pos_t pushup = { .x = 0, .z = 20};
    // YOUR CODE:
    // Loop 'reps' times:
    //   1. Lower all legs to z = 25 (body goes down)
    //      Use move_all_legs() with z = 25
    //      vTaskDelay(pdMS_TO_TICKS(400))
    //   2. Raise all legs back to z = L2 (body goes up)
    //      vTaskDelay(pdMS_TO_TICKS(400))
    //   Check 'running' between reps
    for(int i= 0; i< 5; i++){

        move_all_legs(pushup, pushup, pushup, pushup);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(200));
        pose_stand();
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    running = false;
    ESP_LOGI(TAG, "Pose: Pushup complete (%d reps)", reps);
}

// ---------------------------------------------------------------------------
// Dance pose (animated - your creativity!)
// ---------------------------------------------------------------------------

static void pose_dance(void)
{
    running = true;
    int cycles = 5;

    for (int c = 0; c < cycles; c++) {
        // --- Beat 1: Body drop + lean right ---
        foot_pos_t drop_left  = { .x = 0, .z = 25 };
        foot_pos_t drop_right = { .x = 0, .z = L2 };
        move_all_legs(drop_left, drop_left, drop_right, drop_right);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(250));

        // --- Beat 2: Pop back up + lean left ---
        foot_pos_t pop_left  = { .x = 0, .z = L2 };
        foot_pos_t pop_right = { .x = 0, .z = 25 };
        move_all_legs(pop_left, pop_left, pop_right, pop_right);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(250));

        // --- Beat 3: Front dip + rear rise (nose down bow) ---
        foot_pos_t front_low = { .x = 5, .z = L2 };
        foot_pos_t rear_high = { .x = -5, .z = 30 };
        move_all_legs(front_low, rear_high, front_low, rear_high);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(250));

        // --- Beat 4: Reverse — rear dip + front rise (butt drop) ---
        foot_pos_t front_high = { .x = -5, .z = 30 };
        foot_pos_t rear_low   = { .x = 5, .z = L2 };
        move_all_legs(front_high, rear_low, front_high, rear_low);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(250));

        // --- Beat 5-6: Shimmy — quick alternating diagonal lifts ---
        foot_pos_t up   = { .x = 0, .z = 30 };
        foot_pos_t down = { .x = 0, .z = L2 };
        // FL+RR up, FR+RL down
        move_all_legs(up, down, down, up);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(150));
        // FL+RR down, FR+RL up
        move_all_legs(down, up, up, down);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(150));
        // Repeat shimmy faster
        move_all_legs(up, down, down, up);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(100));
        move_all_legs(down, up, up, down);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(100));

        // --- Beat 7: FL kicks forward (sass move) ---
        foot_pos_t kick    = { .x = 30, .z = -10 };
        foot_pos_t support = { .x = 0, .z = L2 };
        move_all_legs(kick, support, support, support);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(300));

        // --- Beat 8: Snap back to center + full body drop ---
        foot_pos_t low_all = { .x = 0, .z = 25 };
        move_all_legs(low_all, low_all, low_all, low_all);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(200));

        // --- Beat 9: Pop up tall (the "hey!" moment) ---
        foot_pos_t tall = { .x = 0, .z = L2 };
        move_all_legs(tall, tall, tall, tall);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(400));

        // --- Beat 10: Twist — front legs go right, rear legs go left ---
        foot_pos_t twist_fr = { .x = 15, .z = L2 };
        foot_pos_t twist_fl = { .x = -15, .z = L2 };
        move_all_legs(twist_fl, twist_fr, twist_fr, twist_fl);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(200));

        // --- Beat 11: Twist other way ---
        move_all_legs(twist_fr, twist_fl, twist_fl, twist_fr);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(200));

        // --- Beat 12: Double bounce to reset ---
        foot_pos_t bounce_low  = { .x = 0, .z = 30 };
        foot_pos_t bounce_high = { .x = 0, .z = L2 };
        move_all_legs(bounce_low, bounce_low, bounce_low, bounce_low);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(150));
        move_all_legs(bounce_high, bounce_high, bounce_high, bounce_high);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(150));
        move_all_legs(bounce_low, bounce_low, bounce_low, bounce_low);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(150));
        move_all_legs(bounce_high, bounce_high, bounce_high, bounce_high);
        if (!running) return;
        vTaskDelay(pdMS_TO_TICKS(300));
    }

    // Final pose: stand tall
    pose_stand();

    running = false;
    ESP_LOGI(TAG, "Pose: Dance complete (%d cycles)", cycles);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void pose_execute(pose_id_t pose)
{
    pose_stop();  // Stop any running animation first

    switch (pose) {
        case POSE_STAND:  pose_stand();  break;
        case POSE_SIT:    pose_sit();    break;
        case POSE_LAY:    pose_lay();    break;
        case POSE_WAVE:   pose_wave();   break;
        case POSE_PUSHUP: pose_pushup(); break;
        case POSE_DANCE:  pose_dance();  break;
        default:
            ESP_LOGW(TAG, "Unknown pose: %d", pose);
    }
}

bool pose_is_running(void) { return running; }
void pose_stop(void) { running = false; }

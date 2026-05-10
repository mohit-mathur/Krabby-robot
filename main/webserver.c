/**
 * Web Server - Implementation
 * 
 * HTTP server setup and routing is provided.
 * Handler logic in each handle_api_*() function.
 */

#include <string.h>
#include <stdlib.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"
#include "webserver.h"
#include "web_page.h"
#include "ik.h"
#include "gait.h"
#include "body.h"
#include "poses.h"
#include "oled.h"
#include "servo.h"

static const char *TAG = "WEBSERVER";
static httpd_handle_t server = NULL;

// ---------------------------------------------------------------------------
// Helper: read POST body into buffer
// ---------------------------------------------------------------------------

static int read_post_body(httpd_req_t *req, char *buf, int max_len)
{
    int total_len = req->content_len;
    if (total_len >= max_len) {
        total_len = max_len - 1;
    }
    int received = 0;
    while (received < total_len) {
        int ret = httpd_req_recv(req, buf + received, total_len - received);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue;
            return -1;
        }
        received += ret;
    }
    buf[received] = '\0';
    return received;
}

// ---------------------------------------------------------------------------
// Helper: send JSON response
// ---------------------------------------------------------------------------

static esp_err_t send_json(httpd_req_t *req, cJSON *json)
{
    char *str = cJSON_PrintUnformatted(json);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_sendstr(req, str);
    free(str);
    cJSON_Delete(json);
    return ESP_OK;
}

static esp_err_t send_ok(httpd_req_t *req, const char *message)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "ok");
    cJSON_AddStringToObject(json, "message", message);
    return send_json(req, json);
}

static esp_err_t send_error(httpd_req_t *req, const char *message)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "error");
    cJSON_AddStringToObject(json, "message", message);
    httpd_resp_set_status(req, "400 Bad Request");
    return send_json(req, json);
}

// ---------------------------------------------------------------------------
// GET / — Serve the web UI page
// ---------------------------------------------------------------------------

static esp_err_t handle_root(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, WEB_PAGE_HTML, strlen(WEB_PAGE_HTML));
    return ESP_OK;
}

// ---------------------------------------------------------------------------
//POST /api/move — Walk in a direction
// Body: {"direction": "forward|backward", "speed": 0.0-1.0}
// ---------------------------------------------------------------------------

static esp_err_t handle_api_move(httpd_req_t *req)
{
    char buf[128];
    if (read_post_body(req, buf, sizeof(buf)) < 0) {
        return send_error(req, "Failed to read body");
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json) return send_error(req, "Invalid JSON");

    // Extract fields
    cJSON *dir = cJSON_GetObjectItem(json, "direction");
    cJSON *spd = cJSON_GetObjectItem(json, "speed");

    if (!dir || !cJSON_IsString(dir)) {
        cJSON_Delete(json);
        return send_error(req, "Missing direction");
    }

    float speed = (spd && cJSON_IsNumber(spd)) ? (float)spd->valuedouble : 0.5f;

    // Map direction string to gait mode
    if (strcmp(dir->valuestring, "forward") == 0) {
        gait_set_mode(GAIT_WALK_FORWARD, speed);
    } else if (strcmp(dir->valuestring, "backward") == 0) {
        gait_set_mode(GAIT_WALK_BACKWARD, speed);
    } else if (strcmp(dir->valuestring, "stop") == 0) {
        gait_stop();
    } else {
        cJSON_Delete(json);
        return send_error(req, "Unknown direction");
    }

    cJSON_Delete(json);
    return send_ok(req, "Move command received");
}

// ---------------------------------------------------------------------------
// POST /api/turn — Turn left or right
// Body: {"direction": "left|right", "speed": 0.0-1.0}
// ---------------------------------------------------------------------------

static esp_err_t handle_api_turn(httpd_req_t *req)
{
    char buf[128];
    if (read_post_body(req, buf, sizeof(buf)) < 0) {
        return send_error(req, "Failed to read body");
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json) return send_error(req, "Invalid JSON");

    cJSON *dir = cJSON_GetObjectItem(json, "direction");
    cJSON *spd = cJSON_GetObjectItem(json, "speed");

    if (!dir || !cJSON_IsString(dir)) {
        cJSON_Delete(json);
        return send_error(req, "Missing direction");
    }

    float speed = (spd && cJSON_IsNumber(spd)) ? (float)spd->valuedouble : 0.5f;

    // Map direction string to gait mode
    if (strcmp(dir->valuestring, "left") == 0) {
        gait_set_mode(GAIT_TURN_LEFT, speed);
    } else if (strcmp(dir->valuestring, "right") == 0) {
        gait_set_mode(GAIT_TURN_RIGHT, speed);
    } else if (strcmp(dir->valuestring, "stop") == 0) {
        gait_stop();
    } else {
        cJSON_Delete(json);
        return send_error(req, "Unknown direction");
    }

    cJSON_Delete(json);
    return send_ok(req, "Turn command received");
}

// ---------------------------------------------------------------------------
// POST /api/pose — Execute a preset pose
// Body: {"pose": "stand|sit|lay|wave|pushup|dance"}
// ---------------------------------------------------------------------------

static esp_err_t handle_api_pose(httpd_req_t *req)
{
    char buf[128];
    if (read_post_body(req, buf, sizeof(buf)) < 0) {
        return send_error(req, "Failed to read body");
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json) return send_error(req, "Invalid JSON");


    cJSON *pos = cJSON_GetObjectItem(json, "pose");

    if (!pos || !cJSON_IsString(pos)) {
        cJSON_Delete(json);
        return send_error(req, "Missing Pose");
    }

    // Map direction string to gait mode
    if (strcmp(pos->valuestring, "stand") == 0) {
        pose_execute(POSE_STAND);
    } else if (strcmp(pos->valuestring, "sit") == 0) {
        pose_execute(POSE_SIT);
    } else if (strcmp(pos->valuestring, "lay") == 0) {
        pose_execute(POSE_LAY);
    }
    else if (strcmp(pos->valuestring, "wave") == 0) {
        pose_execute(POSE_WAVE);
    }
    else if (strcmp(pos->valuestring, "pushup") == 0) {
        pose_execute(POSE_PUSHUP);
    } 
    else if (strcmp(pos->valuestring, "dance") == 0) {
        pose_execute(POSE_DANCE);
    }else {
        cJSON_Delete(json);
        return send_error(req, "Unknown direction");
    }

    cJSON_Delete(json);
    return send_ok(req, "Pose command received");
}

// ---------------------------------------------------------------------------
// POST /api/tilt — Body tilt control
// Body: {"pitch": -15 to 15, "roll": -15 to 15}
// ---------------------------------------------------------------------------

static esp_err_t handle_api_tilt(httpd_req_t *req)
{
    char buf[128];
    if (read_post_body(req, buf, sizeof(buf)) < 0) {
        return send_error(req, "Failed to read body");
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json) return send_error(req, "Invalid JSON");

    cJSON *pit = cJSON_GetObjectItem(json, "pitch");
    cJSON *rol = cJSON_GetObjectItem(json, "roll");

    if (pit && cJSON_IsNumber(pit)) {
    body_set_pitch((float)pit->valuedouble);
    }
    if (rol && cJSON_IsNumber(rol)) {
        body_set_roll((float)rol->valuedouble);
    }
    body_apply();

    cJSON_Delete(json);
    return send_ok(req, "Tilt applied");
}

// ---------------------------------------------------------------------------
//POST /api/height — Body height control
// Body: {"height": 20 to 47.625}
// ---------------------------------------------------------------------------

static esp_err_t handle_api_height(httpd_req_t *req)
{
    char buf[128];
    if (read_post_body(req, buf, sizeof(buf)) < 0) {
        return send_error(req, "Failed to read body");
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json) return send_error(req, "Invalid JSON");

    cJSON *hgt = cJSON_GetObjectItem(json, "height");
    if (hgt && cJSON_IsNumber(hgt)) {
    body_set_height((float)hgt->valuedouble);
    }

    body_apply();

    cJSON_Delete(json);
    return send_ok(req, "Height set");
}

// ---------------------------------------------------------------------------
// POST /api/mood — Set OLED face mood
// Body: {"mood": "happy|sleepy|angry|surprised|sad|love|idle"}
// ---------------------------------------------------------------------------

static esp_err_t handle_api_mood(httpd_req_t *req)
{
    char buf[128];
    if (read_post_body(req, buf, sizeof(buf)) < 0) {
        return send_error(req, "Failed to read body");
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json) return send_error(req, "Invalid JSON");

    cJSON *mod = cJSON_GetObjectItem(json, "mood");
    
    if(!mod || !cJSON_IsString(mod)){
        cJSON_Delete(json);
        return send_error(req, "Missing mood");

    }
    
    
    if (strcmp(mod->valuestring, "happy") == 0) {
        oled_set_mood(MOOD_HAPPY);}
        else if (strcmp(mod->valuestring, "sleepy") == 0) {
        oled_set_mood(MOOD_SLEEPY);}
        else if (strcmp(mod->valuestring, "angry") == 0) {
        oled_set_mood(MOOD_ANGRY);}
        else if (strcmp(mod->valuestring, "surprised") == 0) {
        oled_set_mood(MOOD_SURPRISED);}
        else if (strcmp(mod->valuestring, "sad") == 0) {
        oled_set_mood(MOOD_SAD);}
        else if (strcmp(mod->valuestring, "love") == 0) {
        oled_set_mood(MOOD_LOVE);}
        else if (strcmp(mod->valuestring, "idle") == 0) {
        oled_set_mood(MOOD_IDLE);}
    else{
        cJSON_Delete(json);
        return send_error(req, "Unknown mood");
    }

    cJSON_Delete(json);
    return send_ok(req, "Mood set");
}

// ---------------------------------------------------------------------------
// POST /api/estop — Emergency stop
// ---------------------------------------------------------------------------

static esp_err_t handle_api_estop(httpd_req_t *req)
{
    gait_stop();
    pose_stop();
    body_set_pitch(0);
    body_set_roll(0);
    body_set_height(L2);
    ik_stand();
    oled_set_mood(MOOD_SURPRISED);
    ESP_LOGW(TAG, "EMERGENCY STOP");
    return send_ok(req, "Emergency stop executed");
}

// ---------------------------------------------------------------------------
// GET /api/status — Return robot state
// Returns: {"mode": "...", "mood": "...", "height": ..., "pitch": ..., "roll": ...}
// ---------------------------------------------------------------------------

static esp_err_t handle_api_status(httpd_req_t *req)
{
   
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "ok");

    cJSON_AddNumberToObject(json, "mode", gait_get_mode());
    cJSON_AddNumberToObject(json, "mood", oled_get_mood());
    cJSON_AddNumberToObject(json, "height", body_get_height());
    cJSON_AddNumberToObject(json, "pitch", body_get_pitch());
    cJSON_AddNumberToObject(json, "roll", body_get_roll());

    cJSON *servos = cJSON_CreateArray();
    for(int i = 0; i< 8; i++){
        cJSON_AddItemToArray(servos, cJSON_CreateNumber(servo_get_angle(i)));
    }
    cJSON_AddItemToObject(json, "servos", servos);

    // ADD MORE FIELDS HERE
    return send_json(req, json);
}

// ---------------------------------------------------------------------------
// CORS preflight handler (needed for browser fetch requests)
// ---------------------------------------------------------------------------

static esp_err_t handle_options(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// ---------------------------------------------------------------------------
// Route registration
// ---------------------------------------------------------------------------

static const httpd_uri_t routes[] = {
    { .uri = "/",            .method = HTTP_GET,     .handler = handle_root },
    { .uri = "/api/move",    .method = HTTP_POST,    .handler = handle_api_move },
    { .uri = "/api/turn",    .method = HTTP_POST,    .handler = handle_api_turn },
    { .uri = "/api/pose",    .method = HTTP_POST,    .handler = handle_api_pose },
    { .uri = "/api/tilt",    .method = HTTP_POST,    .handler = handle_api_tilt },
    { .uri = "/api/height",  .method = HTTP_POST,    .handler = handle_api_height },
    { .uri = "/api/mood",    .method = HTTP_POST,    .handler = handle_api_mood },
    { .uri = "/api/estop",   .method = HTTP_POST,    .handler = handle_api_estop },
    { .uri = "/api/status",  .method = HTTP_GET,     .handler = handle_api_status },
    // CORS preflight
    { .uri = "/api/move",    .method = HTTP_OPTIONS, .handler = handle_options },
    { .uri = "/api/turn",    .method = HTTP_OPTIONS, .handler = handle_options },
    { .uri = "/api/pose",    .method = HTTP_OPTIONS, .handler = handle_options },
    { .uri = "/api/tilt",    .method = HTTP_OPTIONS, .handler = handle_options },
    { .uri = "/api/height",  .method = HTTP_OPTIONS, .handler = handle_options },
    { .uri = "/api/mood",    .method = HTTP_OPTIONS, .handler = handle_options },
    { .uri = "/api/estop",   .method = HTTP_OPTIONS, .handler = handle_options },
};

void webserver_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 20;
    config.stack_size = 8192;

    if (httpd_start(&server, &config) == ESP_OK) {
        int n = sizeof(routes) / sizeof(routes[0]);
        for (int i = 0; i < n; i++) {
            httpd_register_uri_handler(server, &routes[i]);
        }
        ESP_LOGI(TAG, "Web server started with %d routes", n);
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
    }
}

void webserver_stop(void)
{
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

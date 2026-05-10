idf_component_register(
    SRCS
        "main.c"
        "servo.c"
        "ik.c"
        "gait.c"
        "body.c"
        "poses.c"
        "oled.c"
        "wifi.c"
        "webserver.c"
    INCLUDE_DIRS "."
    REQUIRES
        driver
        esp_wifi
        esp_http_server
        esp_netif
        nvs_flash
        esp_event
        esp_hal_ledc
        esp_driver_ledc

)

#ifndef ESP32_DEMO_OTA_H
#define ESP32_DEMO_OTA_H

#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/7/29 15:56
* @description 固件OTA空中升级
*/

void exec_ota();

esp_err_t do_firmware_upgrade(void *pvParameter);

void check_ota();

#endif

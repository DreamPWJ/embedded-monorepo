#ifndef EMBEDDED_MONOREPO_OTA_H
#define EMBEDDED_MONOREPO_OTA_H

#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/7/29 15:56
* @description 固件OTA空中升级
*/

void exec_ota(String version, String jsonUrl);

void do_firmware_upgrade(String version, String jsonUrl, String firmwareUrl = "");

void x_task_ota(void *pvParameters);


#endif

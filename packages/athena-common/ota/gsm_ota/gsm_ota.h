#ifndef EMBEDDED_MONOREPO_GSM_OTA_H
#define EMBEDDED_MONOREPO_GSM_OTA_H

#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/9/16 13:59
* @description GSM网络OTA空中升级
*/

void do_gsm_firmware_upgrade(String version, String jsonUrl);

void gsm_exec_ota(String version, String jsonUrl);

void do_gsm_at_diff_firmware_upgrade();

#endif

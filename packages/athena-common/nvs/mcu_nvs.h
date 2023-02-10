#ifndef EMBEDDED_MONOREPO_NVS_H
#define EMBEDDED_MONOREPO_NVS_H

#include <Arduino.h>


/**
* @author 潘维吉
* @date 2022/9/3 17:05
* @description 单片机持久化存储 非易失性存储
* 文档地址： https://github.com/rpolitex/ArduinoNvs
*/


void int_nvs();

String get_nvs(String key);

void set_nvs(String key, String data);

bool is_key_nvs(String key);

bool clear_nvs();

#endif

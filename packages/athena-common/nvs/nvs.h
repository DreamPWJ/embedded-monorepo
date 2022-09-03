#ifndef EMBEDDED_MONOREPO_NVS_H
#define EMBEDDED_MONOREPO_NVS_H

#include <Arduino.h>


/**
* @author 潘维吉
* @date 2022/9/3 17:05
* @description 单片机持久化存储 非易失性存储
* 文档地址： https://github.com/rpolitex/ArduinoNvs
*/


void intNVS();

String getNVS(String key);

bool setNVS(String key, String data);


#endif

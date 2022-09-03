#include "nvs.h"
#include <Arduino.h>
#include <ArduinoNvs.h>

/**
* @author 潘维吉
* @date 2022/9/3 17:05
* @description 单片机持久化存储 非易失性存储
*/


void intNVS() {
    NVS.begin();
}

String getNVS(String key) {
    return NVS.getString(key);
}

bool setNVS(String key, String data) {
    return NVS.setString(key, data);
}
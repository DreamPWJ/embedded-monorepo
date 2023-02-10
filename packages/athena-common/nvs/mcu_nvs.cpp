#include "mcu_nvs.h"
#include <Arduino.h>
#include <Preferences.h>  // Preferences 库是 arduino-esp32 独有的NVS库

/**
* @author 潘维吉
* @date 2022/9/3 17:05
* @description 单片机Flash持久化存储 非易失性存储NVS
* 参考文档: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/preferences.html
*/

Preferences nvsPref; // 声明对象名

void int_nvs() {
    Serial.println("初始化非易失性存储NVS");
    nvsPref.begin("esp32nvs", false); // 打开命名空间存储分片名称 false读写权限 true只读 默认false
}

String get_nvs(String key) {
    if (is_key_nvs(key)) {
        return nvsPref.getString(key.c_str(), "");
    }
    return "";
}

/**
 * key关键字与系统默认内置关键字冲突 会导致存储失败
 */
bool set_nvs(String key, String data) {
    return nvsPref.putString(key.c_str(), data.c_str());
}

/**
 * 是否存在key关键字
 */
bool is_key_nvs(String key) {
    return nvsPref.isKey(key.c_str());
}

/**
 * 清空所有的NVS键和值
 */
bool clear_nvs() {
    return nvsPref.clear();
}
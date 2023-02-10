#include "mcu_nvs.h"
#include <Arduino.h>
#include <Preferences.h>  // Preferences 库是 arduino-esp32 独有的NVS库

/**
* @author 潘维吉
* @date 2022/9/3 17:05
* @description 单片机Flash持久化存储 非易失性存储NVS
* 参考文档: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/preferences.html
*/

Preferences nvs; //声明对象名

void int_nvs() {
    nvs.begin("esp32-nvs", false); // 打开命名空间名称 false读写权限 true只读 默认false
}

String get_nvs(String key) {
    return nvs.getString(key.c_str());
}

/**
 * key关键字与系统默认内置关键字冲突 会导致存储失败
 */
bool set_nvs(String key, String data) {
    return nvs.putString(key.c_str(), data);
}

/**
 * 清空所有的NVS键和值
 */
bool clear_nvs() {
    return nvs.clear();
}
#include "mcu_nvs.h"
#include <Arduino.h>
#include <Preferences.h>  // Preferences 库是 arduino-esp32 独有的NVS库
#include <nvs_flash.h>

/**
* @author 潘维吉
* @date 2022/9/3 17:05
* @description 单片机Flash持久化存储 非易失性存储NVS
* 参考文档: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/preferences.html
* https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
*/

Preferences preferences; // 声明对象名
// nvs_handle_t my_handle;

void int_nvs() {
    Serial.println("初始化非易失性存储NVS");
    //nvs_flash_erase(); // erase the NVS partition and...
    //nvs_flash_init(); // initialize the NVS partition.
    // Serial.println(nvsPref.freeEntries()); // 获取空余nvs空间
    preferences.begin("flashdata", false); // 打开命名空间存储分片名称 false读写权限 true只读 默认false
    /*  nvs_flash_init();
      nvs_open("storage", NVS_READWRITE, &my_handle);*/
}

String get_nvs(String key) {
/*    size_t required_size = 0;
    nvs_get_str(my_handle, key.c_str(), NULL, &required_size);
    char *server_name = static_cast<char *>(malloc(required_size));
    nvs_get_str(my_handle, key.c_str(), server_name, &required_size);
    printf("Read data: %s\n", server_name);
    return server_name;*/

    if (is_key_nvs(key) == true) {
        String result = preferences.getString(key.c_str(), "");
        return result;
    }
    return "";
}

/**
 * key关键字与系统默认内置关键字冲突 会导致存储失败
 */
void set_nvs(String key, String data) {
/*    nvs_set_str(my_handle, key.c_str(), data.c_str());
    printf("Write data: %s\n", data.c_str());
    nvs_commit(my_handle);
    nvs_close(my_handle);*/

    preferences.putString(key.c_str(), data.c_str());
}

/**
 * 是否存在key关键字
 */
bool is_key_nvs(String key) {
    return preferences.isKey(key.c_str());
}

/**
 * 清空所有的NVS键和值
 */
bool clear_nvs() {
    return preferences.clear();
}
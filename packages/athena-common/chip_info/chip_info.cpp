#include "chip_info.h"
#include <Arduino.h>
#include "esp_system.h"
#include "esp_spi_flash.h"

/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 芯片信息模块
*/


/**
 * 获取芯片信息
 */
void get_chip_info() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
}
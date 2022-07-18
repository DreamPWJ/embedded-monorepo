#include <Arduino.h>
#include "esp_system.h"
#include "esp_spi_flash.h"

// LED等引脚名称数字 开发板有标注 如果不是 Arduino 框架定义的，则设置
#ifndef LED_PIN
#define LED_PIN 18
#endif

void setup() {
// write your initialization code here
    Serial.begin(9600);

    //将 LED 数字引脚初始化为输出
    pinMode(LED_PIN, OUTPUT);
}


/* Print chip information */
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

void loop() {
// write your code here
    Serial.print("Hello World Embedded ESP32! \n");
    // 获取硬件信息
    // get_chip_info();

    delay(1000);

    // 开发板LED 闪动的实现
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
}
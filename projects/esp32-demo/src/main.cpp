#include <Arduino.h>
#include <BLEAdvertising.h>
#include <BLEDevice.h>
#include "esp_system.h"
#include "esp_spi_flash.h"

// LED等引脚名称数字 开发板有标注 如果不是 Arduino 框架定义的，则设置
#ifndef LED_PIN
#define LED_PIN 18
#endif

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

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

/* 初始化蓝牙5.0 */
void initBLE() {
    Serial.println("开始蓝牙广播模式...");

    BLEDevice::init("ESP32-PanWeiJi");

    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic->setValue("Hello World says Neil");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("蓝牙已就绪, 等待一个客户端连接...");

}

void setup() {
// write your initialization code here
    Serial.begin(115200);

    // 将 LED 数字引脚初始化为输出
    pinMode(LED_PIN, OUTPUT);

    // 初始化蓝牙
    initBLE();
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
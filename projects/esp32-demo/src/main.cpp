#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "../lib/bluetooth_low_energy/bluetooth_low_energy.h"

// LED等引脚名称数字 开发板有标注 如果不是 Arduino 框架定义的，则设置
#ifndef LED_PIN
#define LED_PIN 18
#endif

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "e57997fe-066f-11ed-b939-0242ac120002"
#define CHARACTERISTIC_UUID "ee3b5210-066f-11ed-b939-0242ac120002"

static BLECharacteristic *characteristic;
static BLEAdvertising *advertising;

uint8_t devicesConnected = 0;

// 服务器回调
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *server) {
        Serial.println("蓝牙连接成功");
        devicesConnected++;
        advertising->start();
    }

    void onDisconnect(BLEServer *server) {
        Serial.println("蓝牙连接取消");
        devicesConnected--;
    }
};

// 特性回调
class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
        Serial.println("onWrite");
        std::string value = characteristic->getValue();

        Serial.println(value.c_str());
    }

    void onRead(BLECharacteristic *characteristic) {
        Serial.println("onRead");
        characteristic->setValue("Hello");
    }
};

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

/* 初始化蓝牙设置 */
void initBLE(String bleName) {
    // 参考文档: https://github.com/Nicklason/esp32-ble-server/blob/master/src/main.cpp
    Serial.println("开始初始化蓝牙...");
    BLEDevice::init(bleName.c_str()); //初始化一个蓝牙设备  将传入的BLE的名字转换为指针

    BLEServer *server = BLEDevice::createServer(); //创建一个蓝牙服务器
    server->setCallbacks(new ServerCallbacks()); //服务器回调函数设置为ServerCallbacks
    BLEService *service = server->createService(SERVICE_UUID); //创建一个BLE服务

    characteristic = service->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ |
                                                                        BLECharacteristic::PROPERTY_WRITE |
                                                                        BLECharacteristic::PROPERTY_NOTIFY |
                                                                        BLECharacteristic::PROPERTY_INDICATE);
    characteristic->addDescriptor(new BLE2902());
    characteristic->setCallbacks(new CharacteristicCallbacks());

    service->start(); // 开启蓝牙服务
    advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
    advertising->start(); // 蓝牙服务器开始广播

    Serial.println("蓝牙已就绪, 等待一个客户端连接...");

}

void setup() {
// write your initialization code here
    Serial.begin(115200);

    // 将 LED 数字引脚初始化为输出
    pinMode(LED_PIN, OUTPUT);

    // 初始化蓝牙设置
    initBLE("ESP32-PanWeiJi");
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
    initSetBLE();

//  蓝牙设置
/*    if (devicesConnected > 0) {
        Serial.println("Notifying devices");
        characteristic->setValue("Hello connected devices!");
        characteristic->notify();
    } */
}
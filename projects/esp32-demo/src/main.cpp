#include <Arduino.h>
#include <BLEAdvertising.h>
#include <BLEDevice.h>
#include "esp_system.h"
#include "esp_spi_flash.h"

// LED等引脚名称数字 开发板有标注 如果不是 Arduino 框架定义的，则设置
#ifndef LED_PIN
#define LED_PIN 18
#endif

BLECharacteristic *pCharacteristic; //创建一个BLE特性pCharacteristic
bool deviceConnected = false;       //连接否标志位
uint8_t txValue = 0;                //TX的值
long lastMsg = 0;                   //存放时间的变量
String rxload = "BlackWalnutLabs";  //RX的预置值
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "e57997fe-066f-11ed-b939-0242ac120002"
#define CHARACTERISTIC_UUID "ee3b5210-066f-11ed-b939-0242ac120002"

//服务器回调
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer) {
        deviceConnected = false;
    }
};

//特性回调
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            rxload = "";
            for (int i = 0; i < rxValue.length(); i++) {
                rxload += (char) rxValue[i];
                Serial.print(rxValue[i]);
            }
            Serial.println("");
        }
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
    Serial.println("开始初始化蓝牙...");

    BLEDevice::init(bleName.c_str()); //初始化一个蓝牙设备  将传入的BLE的名字转换为指针

    BLEServer *pServer = BLEDevice::createServer();  // 创建一个蓝牙服务器
    // pServer->setCallbacks(new MyServerCallbacks()); //服务器回调函数设置为MyServerCallbacks
    BLEService *pService = pServer->createService(SERVICE_UUID); //创建一个BLE服务
    //创建一个(写)特征 类型是写入
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE
    );
    //为特征添加一个回调
    //pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Hello World");

    pService->start(); // 开启蓝牙服务
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();  // 蓝牙服务器开始广播

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
}
#include "bluetooth.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 低功耗蓝牙BLE模块
  */

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


/* 初始化设置蓝牙 */
void init_buletooth(String bleName) {
    delay(5000);
    // 参考文档: https://github.com/Nicklason/esp32-ble-server/blob/master/src/main.cpp
    Serial.println("开始初始化蓝牙模块...");
    Serial.println(bleName.c_str());
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

/* 监听蓝牙状态 */
void buletooth_state() {
    if (devicesConnected > 0) {
        Serial.println("Notifying devices");
        characteristic->setValue("Hello connected devices!");
        characteristic->notify();
    }
}

#include "bluetooth_connect.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 低功耗蓝牙BLE模块
*/

#define IS_DEBUG false  // 是否调试模式
#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID           "e57997fe-066f-11ed-b939-0242ac120002"
#define CHARACTERISTIC_UUID_RX "e0f94878-332a-4b77-ab82-67fcb8f31186"
#define CHARACTERISTIC_UUID_TX "a39e356f-31b8-49c4-a19c-f2228e024d40"

static uint8_t txValue = 0;
static BLEServer *pServer = NULL;                   //BLEServer指针 pServer
static BLECharacteristic *pTxCharacteristic = NULL; //BLECharacteristic指针 pTxCharacteristic
static bool deviceConnected = false;                //本次连接状态
static bool oldDeviceConnected = false;             //上次连接状态


/**
 * 蓝牙服务器回调
 */
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        Serial.println("蓝牙连接成功");
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer) {
        Serial.println("蓝牙连接取消");
        deviceConnected = false;
    }
};

/**
 * 蓝牙数据回调
 */
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue(); // 接收信息

        if (rxValue.length() > 0) { // 向串口输出收到的值
            /*        if (rxValue[0] == '1') {
                    }*/
            Serial.print("蓝牙接收信息RX: ");
            for (int i = 0; i < rxValue.length(); i++) {
                Serial.print(rxValue[i]);
            }
            Serial.println();
        }
    }
};

/**
 * 初始化设置蓝牙
 */
void init_bluetooth(String bleName) {
    // 参考文档: https://github.com/Homepea7/ESP32_Code
    Serial.println("开始初始化蓝牙模块...");
    Serial.println(bleName.c_str());
    // 创建一个 BLE 设备
    BLEDevice::init(bleName.c_str());
    // 创建一个 BLE 服务
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); //设置回调
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // 创建一个 BLE 特征
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,
                                                                          BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks()); //设置回调
    pService->start();                  // 开始服务
    pServer->getAdvertising()->start(); // 开始广播
    Serial.println("蓝牙模块启动成功, 等待一个蓝牙客户端连接和接受发送的通知...");

    delay(1000);
    // 蓝牙状态监控
    bluetooth_heart_beat();
}

/**
 * 蓝牙发送数据
 */
void bluetooth_send(String data) {
    pTxCharacteristic->setValue(data.c_str());
    pTxCharacteristic->notify();
    delay(1000);   // 如果有太多包要发送，蓝牙会堵塞
}

/**
 * 监听蓝牙状态
 */
void check_bluetooth_state() {
    // deviceConnected 已连接
    if (deviceConnected) {
        pTxCharacteristic->setValue(&txValue, 1); // 设置要发送的值为1
        pTxCharacteristic->notify();              // 广播
        txValue++;                                // 指针地址自加1
        delay(2000);                              // 如果有太多包要发送，蓝牙会堵塞
    }

    // disconnecting  断开连接
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);                  // 留时间给蓝牙缓冲
        pServer->startAdvertising(); // 重新广播
        Serial.println("开始广播");
        oldDeviceConnected = deviceConnected;
    }

    // connecting  正在连接
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}

/**
 * 多线程MQTT任务
 */
void x_task_bluetooth(void *pvParameters) {
    while (1) {
        // Serial.println("多线程蓝牙任务, 心跳检测...");
        check_bluetooth_state();
        delay(1000 * 60); // 多久执行一次 毫秒
    }
}

/**
 * 蓝牙健康检测
 */
void bluetooth_heart_beat() {
#if !USE_MULTI_CORE

    const char *params = NULL;
    xTaskCreate(
            x_task_bluetooth,  /* Task function. */
            "x_task_bluetooth", /* String with name of task. */
            1024 * 6,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            6,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_bluetooth, "x_task_bluetooth", 1024 * 6, NULL, 6, NULL, 0);
#endif
}

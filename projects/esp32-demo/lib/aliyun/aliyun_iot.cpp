#include "aliyun_iot.h"
#include <AliyunIoTSDK.h>
#include <WiFiClient.h>

/**
* @author 潘维吉
* @date 2022/7/28 11:31
* @description 阿里云物联网IOT平台
* Arduino SDK地址: https://github.com/0xYootou/arduino-aliyun-iot-sdk
*/

// 设置产品和设备的信息，从阿里云设备信息里查看
#define PRODUCT_KEY "a1ATfAdCbzP"
#define DEVICE_NAME "mcu-test-1"
#define DEVICE_SECRET "9ce5d5fe9f7b6ffb40b369e359a8a5df"
#define REGION_ID "cn-shanghai"

static WiFiClient wifiClient;

/**
 * 初始化阿里云IoT SDK
 */
void init_aliyun_iot_sdk() {
    // 初始化 iot，需传入 wifi 的 client，和设备产品信息
    AliyunIoTSDK::begin(wifiClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, REGION_ID);
}
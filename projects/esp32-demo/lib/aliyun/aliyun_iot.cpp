#include "aliyun_iot.h"

#include <WiFiClient.h>
static WiFiClient wifiClient;
#include <AliyunIoTSDK.h>
#include <PubSubClient.h>

/**
* @author 潘维吉
* @date 2022/7/28 11:31
* @description 阿里云物联网IOT平台
* Arduino SDK地址: https://github.com/0xYootou/arduino-aliyun-iot-sdk
 * https://developer.aliyun.com/article/771308?spm=a2c6h.13262185.profile.74.fe895e04u8Jrsw
*/

// 设置产品和设备的信息，从阿里云设备信息里查看
#define PRODUCT_KEY "a1ATfAdCbzP"
#define DEVICE_NAME "mcu-test-1"
#define DEVICE_SECRET "9ce5d5fe9f7b6ffb40b369e359a8a5df"
#define REGION_ID "cn-shanghai"


/**
 * 初始化阿里云IoT SDK
 */
void init_aliyun_iot_sdk() {
    // 初始化 iot，需传入 wifi 的 client，和设备产品信息
    // 在PubSubClient.h文件中关于错误类型的定义：报错MQTT Connect err : -2，说明MQTT没有连接。解决方法是更改PubSubClient.h文件中的两个宏MQTT_MAX_PACKET_SIZE和MQTT_KEEPALIVE，将其改大点，比如改成1024和60
    AliyunIoTSDK::begin(wifiClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, REGION_ID);
}
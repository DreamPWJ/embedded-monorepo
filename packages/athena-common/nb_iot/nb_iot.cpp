#include "nb_iot.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <hex_utils.h>
#include <json_utils.h>
#include <mcu_nvs.h>
#include <iostream>
#include <string>
#include <common_utils.h>
#include <at_mqtt/at_mqtt.h>
#include <chip_info.h>

using namespace std;

#define IS_DEBUG false // 是否调试模式

/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description nb-iot物联网网络协议
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

// NB-IoT控制GPIO
#define MODEM_RST  6

/**
 * 初始化NB网络协议
 */
void init_nb_iot() {
    // NB相关引脚初始化
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);

    String isNBInit = get_nvs("is_nb_iot_init");

    // if (isNBInit.c_str() != "yes") {  // 如果NB-IOT配网成功 重启等会自动入网 只需初始化一次
    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线  参考文章: https://aithinker.blog.csdn.net/article/details/120765734
    restart_nb_iot();
    Serial.println("给NB-IoT模组发送AT指令, 配置网络...");

    // myNBSerial.printf("AT\r\n"); // 测试AT指令
    // send_at_command("AT+ECICCID\r\n", 5000, IS_DEBUG); // 查看SIM ID号
    send_at_command("AT+CGATT=1\r\n", 60000, IS_DEBUG); //  附着网络  CMS ERROR:308物联网卡被锁(换卡或解锁),没信号会导致设置失败
    send_at_command("AT+CGDCONT=1,\042IP\042,\042CMNBIOT1\042\r\n", 60000,
                    IS_DEBUG); // 注册APNID接入网络 如CMNET,  NB-IOT通用类型CMNBIOT1, CMS ERROR:3附着不成功或没装卡
    send_at_command("AT+CGACT=1\r\n", 10000, IS_DEBUG); // 激活网络
    send_at_command("AT+CREG=1\r\n", 10000, IS_DEBUG); // 注册网络
    // send_at_command("AT+ECSNTP=\042210.72.145.44\042,123,0\r\n", 3000, IS_DEBUG); // 同步NTP网络时间 利用SNTP服务器进行UE的本地时间和UTC时间的同步
    // send_at_command("AT+CSQ\r\n", 2000, IS_DEBUG); // 信号质量
    // send_at_command("AT+ECIPR=115200\r\n", 2000, IS_DEBUG); // 设置模组AT串口通信波特率
    //myNBSerial.printf("AT+ECPING=\042www.baidu.com\042\r\n"); // 测试网络
    set_nvs("is_nb_iot_init", "yes"); // 单片机持久化存储是否初始化NB-IoT网络
    //  }

    // NB模块心跳检测网络
#if !USE_MULTI_CORE
    xTaskCreate(
            nb_iot_heart_beat,
            "nb_iot_heart_beat",
            1024 * 2,
            NULL,
            2,
            NULL);
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(nb_iot_heart_beat, "nb_iot_heart_beat", 8192, NULL, 5, NULL, 0);
#endif

}

/**
 * 发送AT指令
 */
String send_at_command(String command, const int timeout, boolean isDebug, String successResult) {
    String response = "";
    Serial1.print(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
        while (Serial1.available()) {
            char c = Serial1.read();
            response += c;
        }
        if (response.indexOf(successResult) != -1) { // 获取到成功结果 退出循环
            break;
        }
        delay(10);
    }
    if (isDebug) {
        Serial.println(command + "AT指令响应数据: " + response);
    }
    return response;
}

/**
 * AT指令响应数据
 */
void at_command_response() {
    while (Serial1.available()) {
        Serial.println(Serial1.readStringUntil('\n'));
    }
}

/**
 * NB模块心跳检测网络
 */
void nb_iot_heart_beat(void *pvParameters) {
    while (1) {
        Serial1.printf("AT+CSQ\r\n");  // 获取信号质量 如RSSI
        delay(3000);
        String networkRSSI = get_nvs("network_rssi"); // 信号质量
        vector<string> dataArray = split(networkRSSI.c_str(), ",");
        String rssi = dataArray[0].c_str();
        // Serial.println(rssi);
        if (rssi.c_str() == "+CSQ: 0" || rssi.c_str() == "+CSQ: 1") { // 信号丢失重连机制
            // 重新初始化网络
            init_nb_iot();
            init_at_mqtt();

            string chip_id = to_string(get_chip_mac());
            DynamicJsonDocument doc(200);
            doc["type"] = "reconnectNBIoT";
            doc["msg"] = "检测重连NB-IoT网络服务完成: " + chip_id + "单片机发布的消息";
            String initStr;
            serializeJson(doc, initStr);
            std::string mcu_topic = "ESP32/common";
            at_mqtt_publish(mcu_topic.c_str(), initStr.c_str());
        }
        delay(1000 * 60);
    }
}

/**
 * 重启NB模块芯片
 */
void restart_nb_iot() {
    Serial.println("重启GSM调制解调器模块芯片...");
    Serial1.printf("AT+ECRST\r\n"); // 重启NB模块芯片
    set_nvs("is_nb_iot_init", "no");
    delay(2000);
}

/**
 * 获取国际移动设备唯一标识IMEI串号码
 */
String get_imei() {
    // return modem.getIMEI();
    Serial1.printf("AT+CGSN=1\r\n");  // 取产品序列号IMEI
    return "";
}
#include "nb_iot.h"
#include <Arduino.h>
#include <ArduinoJson.h>
//#include <MKRGSM.h>
#include <SoftwareSerial.h>
#include <hex_utils.h>
#include <json_utils.h>
#include <mcu_nvs.h>
#include <iostream>
#include <string>
#include <common_utils.h>
#include <at_mqtt/at_mqtt.h>

using namespace std;

#define DEBUG true

#define PIN_RX 19
#define PIN_TX 18

// Set up a new SoftwareSerial object
SoftwareSerial myNBSerial(PIN_RX, PIN_TX);
//SoftwareSerial myNBSerial;

/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description nb-iot物联网网络协议
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

// modem verification object
// gsmmodem modem;

// NB-IoT控制GPIO
#define MODEM_RST             6
//#define MODEM_PWKEY          10

/**
 * 初始化NB网络协议
 */
void init_nb_iot() {
    // NB相关引脚初始化
    pinMode(MODEM_RST, OUTPUT);
    //pinMode(MODEM_PWKEY, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
    //digitalWrite(MODEM_PWKEY, HIGH);

    pinMode(PIN_RX, INPUT);
    pinMode(PIN_TX, OUTPUT);
    // 参考文档： https://github.com/plerup/espsoftwareserial
    //myNBSerial.begin(9600, SWSERIAL_8N1, PIN_RX, PIN_TX, false); // NB模组的波特率
    myNBSerial.begin(9600);
    if (!myNBSerial) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid SoftwareSerial pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }
    String isNBInit = get_nvs("is_nb_iot_init");
    // Serial.println(isNBInit);
    //  if (isNBInit.c_str() == "yes") {  // 如果NB-IOT配网成功 重启等会自动入网 只需初始化一次
    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线  参考文章: https://aithinker.blog.csdn.net/article/details/120765734
    restart_nb_iot();
    Serial.println("给NB-IoT模组发送AT指令, 配置网络...");
    // myNBSerial.printf("AT\r\n"); // 测试AT指令
    // send_at_command("AT+ECICCID\r\n", 5000, DEBUG); // 查看SIM ID号

    send_at_command("AT+CGATT=1\r\n", 6000, DEBUG); // // 附着网络  CMS ERROR:308物联网卡被锁(换卡或解锁),没信号会导致设置失败
    send_at_command("AT+CGDCONT=1,\042IP\042,\042CMNBIOT1\042\r\n", 8000,
                    DEBUG); // 注册APNID接入网络 如CMNET,  NB-IOT通用类型CMNBIOT1, CMS ERROR:3附着不成功或没装卡
    send_at_command("AT+CGACT=1\r\n", 3000, DEBUG); // 激活网络
    send_at_command("AT+CREG=1\r\n", 3000, DEBUG); // 注册网络
    send_at_command("AT+CSQ\r\n", 2000, DEBUG); // 信号质量
    // send_at_command("AT+ECIPR=115200\r\n", 2000, DEBUG); // 设置模组AT串口通信波特率
    //myNBSerial.printf("AT+ECPING=\042www.baidu.com\042\r\n"); // 测试网络
    set_nvs("is_nb_iot_init", "yes"); // 单片机持久化存储是否初始化NB-IoT网络
    //   }

    // NB模块心跳检测网络
    // nb_iot_heart_beat();
#if !USE_MULTI_CORE
    xTaskCreate(
            nb_iot_heart_beat,
            "nb_iot_heart_beat",
            8192,
            NULL,
            7,
            NULL);
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(nb_iot_heart_beat, "nb_iot_heart_beat", 8192, NULL, 5, NULL, 0);
#endif

}

/**
 * 发送AT指令
 */
String send_at_command(String command, const int timeout, boolean debug, String successResult) {
    String response = "";
    myNBSerial.print(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
        while (myNBSerial.available()) {
            char c = myNBSerial.read();
            response += c;
        }
        if (response.indexOf(successResult) != -1) { // 获取到成功结果 退出循环
            break;
        }
    }
    if (debug) {
        Serial.println(command + "AT指令响应数据: " + response);
    }
    return response;
}

/**
 * AT指令响应数据
 */
void at_command_response() {
    while (myNBSerial.available()) {
        Serial.println(myNBSerial.readStringUntil('\n'));
    }
}

/**
 * NB模块心跳检测网络
 */
void nb_iot_heart_beat(void *pvParameters) {
    /*   unsigned long tm = millis();
       while (millis() - tm <= 10000) { // myNBSerial.available()
           myNBSerial.printf("AT+CREG?\r\n"); // 查询命令返回当前网络注册状态
           delay(1000);
           myNBSerial.printf("AT+CSQ\r\n"); // 获取信号质量 如RSSI
           delay(1000);
           // 等待数据返回结果
           String flag1 = "+CME ERROR:";
           String flag2 = "+CSQ";
           String incomingByte;
           incomingByte = myNBSerial.readString();
           Serial.println(incomingByte);
           if (incomingByte.indexOf(flag1) != -1) {
               // 心跳检测NB网络 异常重启NB模块芯片
               init_nb_iot();
           } else if (incomingByte.indexOf(flag2) != -1) {
               vector<string> dataArray = split(incomingByte.c_str(), ",");
               String rssi = dataArray[0].c_str();
               Serial.println(rssi);
               // NVS存储信号信息 用于MQTT上报
               set_nvs("network_rssi", rssi);
               const char *topics = "ESP32/common";
               at_mqtt_publish(topics, rssi);
           }

       }*/
    while (1) {
        delay(1000 * 20);
        String incomingByte = send_at_command("AT+CSQ\r\n", 10000, DEBUG, "+CSQ");  // 获取信号质量 如RSSI
        const char *topics = "ESP32/common";
        if (incomingByte.indexOf("+CSQ:") != -1) {
            vector<string> dataArray = split(incomingByte.c_str(), ",");
            String rssi = dataArray[0].c_str();
            Serial.println(rssi);
            // NVS存储信号信息 用于MQTT上报
            set_nvs("network_rssi", rssi);
            at_mqtt_publish(topics, rssi);
        }

    }

}

/**
 * 重启NB模块芯片
 */
void restart_nb_iot() {
    Serial.println("重启GSM调制解调器模块芯片...");
    myNBSerial.printf("AT+ECRST\r\n"); // 重启NB模块芯片
    set_nvs("is_nb_iot_init", "no");
    delay(2000);
}

/**
 * 获取国际移动设备唯一标识IMEI串号码
 */
String get_imei() {
    // return modem.getIMEI();
    myNBSerial.printf("AT+CGSN=1\r\n");  // 取产品序列号IMEI
    return "";
}
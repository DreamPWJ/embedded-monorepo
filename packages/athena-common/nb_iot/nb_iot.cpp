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


/**
* @author 潘维吉
* @date 2022/8/22 14:44
* @description nb-iot物联网网络协议
*/

#define IS_DEBUG false // 是否调试模式
#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定
#define MODEM_RST  6  // NB-IoT控制GPIO
#define PIN_RX 19
#define PIN_TX 18

/**
 * 初始化NB网络协议
 */
void init_nb_iot() {
    // NB相关引脚初始化
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);

    Serial1.begin(9600, SERIAL_8N1, PIN_RX, PIN_TX);
    if (!Serial1) { // If the object did not initialize, then its configuration is invalid
        Serial.println("Invalid Serial1 pin configuration, check config");
        while (1) { // Don't continue with invalid configuration
            Serial.print(".");
            delay(1000);
        }
    }

    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线  参考文章: https://aithinker.blog.csdn.net/article/details/120765734
    // restart_nb_iot();
    Serial.println("给NB-IoT模组发送AT指令, 配置网络...");

    // Serial1.printf("AT\r\n"); // 测试AT指令
    // send_at_command("AT+ECICCID\r\n", 5000, IS_DEBUG); // 查看SIM ID号

    String isNBInit = get_nvs("is_nb_iot_init");
    if (isNBInit != "yes") {
        Serial.println("如果NB-IOT配网成功 重启等会自动入网 只需初始化一次");
        send_at_command("AT+CGATT=1\r\n", 30000, IS_DEBUG); //  附着网络  CMS ERROR:308物联网卡被锁(换卡或解锁),没信号会导致设置失败
        send_at_command("AT+CGDCONT=1,\042IP\042,\042CMNET\042\r\n", 30000,
                        IS_DEBUG); // 注册APNID接入网络 如CMNET,  NB-IOT通用类型CMNBIOT1, CMS ERROR:3附着不成功或没装卡
    } else {
        // delay(3000); //  附着网络等可能长达2分钟才成功
        // +CSQ: 99,99 已经读取不到信号强度，搜寻NBIOT网络中
        String flag = "+CSQ: 99,99";
        while (1) {
            String atResult = send_at_command("AT+CSQ\r\n", 3000, IS_DEBUG);
            if (atResult.indexOf(flag) != -1) {
                delay(3000);
            } else {
                break;
            }
        }

        // send_at_command("AT+CGATT?\r\n", 60000, IS_DEBUG, "+CGATT: 1");
        // 判断附着网络是否成功  第二个参数1或5标识附着正常 如 +CEREG: 0,1
/*      String atResult = send_at_command("AT+CEREG?\r\n", 60000, IS_DEBUG);
        String flag = "+CEREG:";
        int startIndex = atResult.indexOf(flag);
        String start = atResult.substring(startIndex);
        int endIndex = start.indexOf("\n");
        String end = start.substring(0, endIndex + 1);
        String data = end.substring(0, end.length());
        vector<string> dataArray = split(data.c_str(), ",");
        String reg = dataArray[1].c_str();
        if (reg.c_str() != "1" && reg.c_str() != "5") {
            Serial.println("NB-IOT附着网络失败重试...");
            init_nb_iot();
        }*/
    }

    send_at_command("AT+CGACT=1\r\n", 5000, IS_DEBUG); // 激活网络
    // send_at_command("AT+CREG=1\r\n", 5000, IS_DEBUG); // 注册网络

    // send_at_command("AT+ECSNTP=\042210.72.145.44\042,123,0\r\n", 3000, IS_DEBUG); // 同步NTP网络时间 利用SNTP服务器进行UE的本地时间和UTC时间的同步
    // send_at_command("AT+CSQ\r\n", 2000, IS_DEBUG); // 信号质量
    // send_at_command("AT+ECIPR=115200\r\n", 2000, IS_DEBUG); // 设置模组AT串口通信波特率
    // Serial1.printf("AT+ECPING=\042www.baidu.com\042\r\n"); // 测试网络
    set_nvs("is_nb_iot_init", "yes"); // 单片机持久化存储是否初始化NB-IoT网络

    delay(1000);
    // NB模块心跳检测网络
#if !USE_MULTI_CORE
    xTaskCreate(
            nb_iot_heart_beat,
            "nb_iot_heart_beat",
            1024 * 8,
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
/*     delay(3000);
        String networkRSSI = get_nvs("network_rssi"); // 信号质量
        vector<string> dataArray = split(networkRSSI.c_str(), ",");
        String rssi = dataArray[0].c_str();
        // Serial.println(rssi);
        if (rssi.c_str() == "+CSQ: 99" || rssi.c_str() == "+CSQ: 0" || rssi.c_str() == "+CSQ: 1") { // 信号丢失重连机制
            Serial.println("NB-IoT信号丢失重连机制...");
            // 重新初始化网络
            restart_nb_iot();
            init_nb_iot();
            init_at_mqtt();
        }*/
        delay(1000 * 60);
    }
}

/**
 * 重启NB模块芯片
 */
void restart_nb_iot() {
    Serial.println("重启GSM调制解调器模块芯片...");
    send_at_command("AT+ECRST\r\n", 60000, IS_DEBUG); // 重启NB模块芯片
    set_nvs("is_nb_iot_init", "no");
}

/**
 * 获取国际移动设备唯一标识IMEI串号码
 */
String get_imei() {
    // return modem.getIMEI();
    Serial1.printf("AT+CGSN=1\r\n");  // 取产品序列号IMEI
    return "";
}
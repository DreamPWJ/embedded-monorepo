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
* @description NB-IoT物联网网络协议
*/

#define IS_DEBUG false // 是否调试模式
#define USE_MULTI_CORE 1 // 是否使用多核 根据芯片决定
#define MODEM_RST  8  // NB-IoT控制GPIO

/**
 * 初始化NB网络协议
 */
void init_nb_iot() {
    // NB-IoT相关引脚初始化
    pinMode(MODEM_RST, OUTPUT); // 确保RX是输入上拉模式 , 否则导致模组不能完全正常工作 返回数据不正确
    digitalWrite(MODEM_RST, LOW);

    // 给NB模组发送AT指令  NB模组出厂自带AT固件 接入天线
    // restart_nb_iot();
    Serial.println("主控单片机向NB-IoT模组发送AT指令, 配置蜂窝网络...");
    delay(3000);

    Serial1.printf("AT\r\n"); // 测试AT指令
    send_at_command("AT+CPSMS=0\r\n", 5000, IS_DEBUG); // 禁用省电模式
    send_at_command("AT+QSCLK=0\r\n", 5000, IS_DEBUG); // 禁用休眠模式

#if IS_DEBUG
    Serial1.println("ATI\r\n"); // 产品固件信息
    delay(1000);
    send_at_command("AT+CPIN?\r\n", 5000, IS_DEBUG); // AT 指令判断模组有没有识别 SIM 卡
    // send_at_command("AT+ECICCID\r\n", 5000, IS_DEBUG); // 查看SIM ID号
#endif

    String isNBInit = get_nvs("is_nb_iot_init");
    if (isNBInit != "yes") {
        Serial.println("初始化NB-IoT配网一次, 以后重启等会自动入网"); // 如果NB-IoT配网成功 重启等会自动入网 只需初始化一次
        send_at_command("AT+CGATT=1\r\n", 30000, IS_DEBUG); // 附着网络 需要先激活SIM卡
        // 注册APN接入网络 如CMNET, NB-IoT通用类型CMNBIOT1 不同的APN类型对功耗省电模式有区别 对下行速率有影响  专网卡需要，不是专网卡不需要配置APN的
        // send_at_command("AT+CGDCONT=1,\042IP\042,\042CMNBIOT1\042\r\n", 30000, IS_DEBUG);
    }
    delay(1000); //  附着网络等可能长达2分钟才成功
    // +CSQ: 99,99 已经读取不到信号强度，搜寻NB-IoT网络中   CSQ信号适合判断2G、3G网络 不适合判断NB网络质量
    String flag = "+CGATT: 1";
    while (1) {
#if IS_DEBUG
        delay(1000);
        Serial1.println("AT+CEREG?\r\n");
        delay(1000);
        Serial1.println("AT+QENG=0\r\n");
        delay(1000);
        Serial1.println("AT+CFUN?\r\n");
        delay(1000);
        Serial1.println("AT+QBAND?\r\n");
        delay(1000);
#endif
        String atResult = send_at_command("AT+CGATT?\r\n", 3000, IS_DEBUG, "+CGATT:");
        // Serial.print(atResult);
        if (atResult.indexOf(flag) != -1) {
            Serial.println("NB-IoT附着网络成功: " + atResult);
            break;
        } else {
            Serial.print(".");
            delay(3000);
        }
    }

    send_at_command("AT+CGACT=1\r\n", 5000, IS_DEBUG); // 激活网络
    send_at_command("AT+CEREG=1\r\n", 5000, IS_DEBUG); // 断网自动上报数据到UART串口  启用上报网络注册状态 URC +CEREG: <stat>

    // 判断网络注册状态 参数1或5标识附着正常
    String flagCEREG1 = "+CEREG: 1,1";
    String flagCEREG5 = "+CEREG: 1,5";
    while (1) {
        String atResultCEREG = send_at_command("AT+CEREG?\r\n", 3000, IS_DEBUG, "+CEREG:");
        // Serial.print(atResultCEREG);
        if (atResultCEREG.indexOf(flagCEREG1) != -1 || atResultCEREG.indexOf(flagCEREG5) != -1) {
            Serial.println("NB-IoT网络连接成功: " + atResultCEREG);
            break;
        } else {
            Serial.print(".");
            delay(3000);
        }
    }

    // send_at_command("AT+ECSNTP=\042210.72.145.44\042,123,0\r\n", 3000, IS_DEBUG); // 同步NTP网络时间 利用SNTP服务器进行UE的本地时间和UTC时间的同步
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
    xTaskCreatePinnedToCore(nb_iot_heart_beat, "nb_iot_heart_beat", 1024 * 8, NULL, 2, NULL, 0);
#endif

}

/**
 * NB模块心跳检测网络
 */
void nb_iot_heart_beat(void *pvParameters) {
    while (1) {
        delay(1000 * 60);
        Serial1.printf("AT+CEREG?\r\n"); // 判断网络注册状态 参数1或5标识附着正常
        delay(1000 * 15);
        Serial1.printf(
                "AT+QMTCONN?\r\n");  // MQTT 服务器连接是否正常  执行 AT+QMTCONN? 时 ， 若当前不存在MQTT连接 ，则无 +QMTCONN: <TCP_connectID>,<state> 返回，仅返回 OK 或者 ERROR。
        delay(1000 * 15);
        Serial1.printf("AT+QENG=0\r\n");  // 模组工程模式 当前的网络服务信息 真正NB-IoT信号质量 走4G LTE部分带宽
        /*delay(1000 * 15);
        Serial1.printf("AT+CSQ\r\n");  // CSQ获取2G网络信号强度 如RSSI*/
    }
}

/**
 * 软重启NB模块芯片
 */
void restart_nb_iot() {
    Serial.println("软重启GSM调制解调器模块芯片...");
    send_at_command("AT+QRST=1\r\n", 30000, IS_DEBUG); // 重启NB模块芯片
    set_nvs("is_nb_iot_init", "no");
}

/**
 * 硬重启NB模块芯片
 */
void hardware_restart_nb_iot() {
    Serial.println("硬重启GSM调制解调器模块芯片...");
    digitalWrite(MODEM_RST, HIGH); // 电平复位
    delay(1000);
    digitalWrite(MODEM_RST, LOW); // 电平工作
    delay(3000);
    set_nvs("is_nb_iot_init", "no");
}

/**
 * 获取NTP时间
 */
String get_nb_ntp_time() {
    // 基于TCP或UDP访问 访问NTP公共服务器获取时间  阿里云NTP服务器是ntp1.aliyun.com（IP为120.25.115.20）端口为123
    // AT+CCLK 设置/获取当前日期和时间
    // send_at_command("AT+CCLK=yyyy-mm-dd hh:mm:ss\r\n", 3000, IS_DEBUG);
    String time = send_at_command("AT+CCLK?\r\n", 3000, IS_DEBUG);
    return time;
}

/**
 * 获取国际移动设备唯一标识IMEI串号码
 */
String get_imei() {
    String atResult = send_at_command("AT+CGSN=1\r\n", 3000, IS_DEBUG); // 取产品序列号IMEI
    // 示例:  +CGSN: 490154203237511
    String flag = "+CGSN:"; // 响应标识
    if (atResult.indexOf(flag) != -1) {
        atResult.replace("+CGSN: ", "");
        return atResult;
    }
    return "";
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
        Serial.println("TX发送的" + command + "指令AT响应数据: " + response);
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
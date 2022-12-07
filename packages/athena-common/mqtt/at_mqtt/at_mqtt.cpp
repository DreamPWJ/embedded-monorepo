#include "at_mqtt.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>
#include <json_utils.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <chip_info.h>
#include <pwm.h>
#include <ground_feeling.h>
#include <common_utils.h>
#include <device_info.h>
#include <ota.h>
#include <mcu_nvs.h>
#include <nb_iot.h>

using namespace std;

/**
* @author 潘维吉
* @date 2022/9/13 15:31
* @description AT指令编写MQTT消息队列遥测传输协议
* 参考文章： https://www.emqx.com/zh/blog/iot-protocols-mqtt-coap-lwm2m
* https://github.com/elementzonline/Arduino-Sample-Codes/tree/master/SIM7600
* https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
*/

#define IS_DEBUG false  // 是否调试模式
#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

String atMqttName = "esp32-mcu-client"; // MQTT客户端前缀名称

const char *at_mqtt_broker = "119.188.90.222"; // 设置MQTT的IP或域名
const char *at_topics = "ESP32/common"; // 设置MQTT的订阅主题
const char *at_mqtt_username = "admin";   // 设置MQTT服务器用户名
const char *at_mqtt_password = "emqx@2022"; // 设置MQTT服务器密码
const int at_mqtt_port = 1883;

/**
 *
 * 初始化MQTT客户端
 */
void init_at_mqtt() {
    Serial.println("初始化MQTT客户端AT指令"); // 确保固件版本支持MQTT

    String client_id = atMqttName + "-";
    string chip_id = to_string(get_chip_mac());
    client_id += chip_id.c_str();   //  String(random(0xffff),HEX); // String(WiFi.macAddress());

    // 执行 AT+QMTOPEN? 时 ，若当前不存在已打开的客户端信息 ，则无 +QMTOPEN:<TCP_connectID>,<host_name>,<port>返回，仅返回 OK 或者 ERROR
    send_mqtt_at_command("AT+QMTCLOSE=0\r\n", 1000, IS_DEBUG);  // 关闭之前的连接 防止再次重连失败

    // 设置MQTT连接所需要的的参数 不同的调制解调器模组需要适配不同的AT指令
    send_mqtt_at_command("AT+QMTCFG=\042version\042,0,1\r\n", 3000, IS_DEBUG);  // 设置MQTT的版本
    //send_mqtt_at_command("AT+ECMTCFG=\042keepalive\042,0,120\r\n", 6000, IS_DEBUG); // 配置心跳时间
    //send_mqtt_at_command("AT+ECMTCFG=\042timeout\042,0,20\r\n", 6000, IS_DEBUG); // 配置数据包的发送超时时间（单位：s，范围：1-60，默认10s）

    // 打开连接  确保网络天线连接正确
    send_mqtt_at_command("AT+QMTOPEN=0,\042" + String(at_mqtt_broker) + "\042," + at_mqtt_port + "\r\n", 15000,
                         IS_DEBUG, "+QMTOPEN: 0,0");  // GSM无法连接局域网, 因为NB本身就是低功耗广域网
/*  myMqttSerial.printf("AT+QMTOPEN=0,\042%s\042,%d\r\n", at_mqtt_broker,
                        at_mqtt_port);  // GSM无法连接局域网, 因为NB本身就是低功耗广域网 */
    String conFlag = "+QMTCONN: 0,0,0"; // 客户端成功连接到 MQTT 服务器返回响应标识
    String connectResult = send_mqtt_at_command(
            "AT+QMTCONN=0,\042" + client_id + "\042,\042" + at_mqtt_username + "\042,\042" + at_mqtt_password +
            "\042\r\n", 15000, IS_DEBUG, conFlag);

    if (connectResult.indexOf(conFlag) != -1) {
        Serial.println("MQTT Broker 连接成功: " + client_id);
    } else {
        Serial.println("MQTT初始化网络连接失败, 自动重启单片机设备...");
        hardware_restart_nb_iot(); // 硬件重启网络模组
        esp_restart();  // 重启单片机主控芯片
    }

    // 发布MQTT消息
    DynamicJsonDocument doc(200);
    doc["type"] = "initMQTT";
    doc["msg"] = "你好, MQTT服务器, 我是" + client_id + "单片机AT指令发布的初始化消息";
    doc["version"] = get_nvs("version");
    String initStr;
    serializeJson(doc, initStr);
    at_mqtt_publish(at_topics, initStr.c_str());
    delay(1000);
    // 订阅MQTT主题消息
    // at_mqtt_subscribe(at_topics);
    std::string topic_device = "ESP32/" + to_string(get_chip_mac()); // .c_str 是 string 转 const char*
    at_mqtt_subscribe(topic_device.c_str()); // 设备单独的主题订阅
    delay(100);
    at_mqtt_subscribe("ESP32/system"); // 系统相关主题订阅

    delay(1000);
    // MQTT心跳服务
    at_mqtt_heart_beat();

}

/**
 * 发送AT指令
 */
String send_mqtt_at_command(String command, const int timeout, boolean isDebug, String successResult) {
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
 * MQTT发布消息
 */
void at_mqtt_publish(String topic, String msg) {
    // 注意完善： 1. 并发队列控制 2. 发送失败重试机制
    // QoS（服务质量）:  0 - 最多分发一次  1 - 至少分发一次  2 - 只分发一次 (保证消息到达并无重复消息) 随着QoS等级提升，消耗也会提升，需要根据场景灵活选择
    Serial1.printf("AT+QMTPUB=0,1,2,0,\042%s\042,%d,\042%s\042\r\n", topic.c_str(), msg.length(), msg.c_str());
    // 获取AT返回的发送是否成功  做重发机制
    // String pubFlag = "OK"; //  MQTT发布消息成功

}

/**
 * MQTT订阅消息
 */
void at_mqtt_subscribe(String topic) {
    Serial1.printf("AT+QMTSUB=0,1,\"%s\",2\r\n", topic.c_str());
}

/**
 * 取消MQTT主题订阅
 */
void at_mqtt_unsubscribe(String topic) {
    Serial1.printf("AT+QMTUNS=0,1,\"%s\"\r\n", topic.c_str());
}

/**
 * MQTT断开连接
 */
void at_mqtt_disconnect() {
    Serial1.printf("AT+QMTDISC=0\\r\\n\r\n");
}

/**
 * 检测重连MQTT服务
 */
void at_mqtt_reconnect(String incomingByte) {
    Serial.println("AT指令重连MQTT服务");
    init_at_mqtt(); // 重连MQTT服务

    DynamicJsonDocument doc(200);
    doc["type"] = "reconnectMQTT";
    doc["msg"] = "检测重连MQTT服务完成: " + to_string(get_chip_mac()) + "单片机发布的消息";
    String initStr;
    serializeJson(doc, initStr);
    at_mqtt_publish(at_topics, initStr.c_str());
}

/**
 * MQTT订阅消息回调
 */
void at_mqtt_callback(String rxData) {
    // Serial.println("AT指令MQTT订阅接收的消息: ");
    // MQTT服务订阅返回AT指令数据格式
    /* +QMTRECV: 0,0,"ESP32/system",{
            "command": "upgrade"
    }*/

    String flagCEREG = "+CEREG:";    // 网络注册状态 判断是否断网
    String flagMQTT = "+QMTRECV:"; // 并发情况下 串口可能返回多条数据  可根据\n\r解析成数组处理多条
    String flagQENG = "+QENG:";    // 模组工程模式 当前的网络服务信息 真正NB-IoT信号质量 走4G LTE部分带宽
    String flagCSQ = "+CSQ:"; // 2G/3G网络信息值
    String flagCONN = "+QMTCONN: 0,4"; // MQTT连接状态 1. 初始化 2. 正在连接  3. 已连接  4. 已断开
    String flagSTAT = "+QMTSTAT:";    // 报告链路层状态 当 MQTT 链路层状态发生变化时，将上报此URC

    //  while (myMqttSerial.available() > 0) { // 串口缓冲区有数据 数据长度
    /*
      Serial.println("因为NB-IoT窄带宽蜂窝网络为半双工 导致MQTT消息发布和订阅不能同时 此处做延迟处理");
      delay(200);
    */
    // yield(); // 专用于主动调用运行后台。 在ESP单片机实际运行过程中，有时会不可避免需要长时间延时，这些长时间延时可能导致单线程的C/C++后台更新不及时，会导致看门狗触发 可使用yield()；主动调用后台程序防止重启。
    String incomingByte = rxData; // 全部串口数据
    // incomingByte = myMqttSerial.readString();
#if IS_DEBUG
    Serial.println("------------------MQTT------------------");
    Serial.println(incomingByte);
    Serial.println("******************MQTT******************");
#endif

    vector<string> rxDataArray = split(incomingByte.c_str(), "\\n");
    for (int i = 0; i < rxDataArray.size(); i++) {  // 并发数据处理
        String incomingByteItem = rxDataArray[i].c_str();

        // MQTT订阅回调消息
        if (incomingByteItem.indexOf(flagMQTT) != -1) {
#if IS_DEBUG
            std::string topic_device = "ESP32/" + to_string(get_chip_mac()); // .c_str 是 string 转 const char*
            at_mqtt_publish(topic_device.c_str(), incomingByteItem.c_str());  // 上报MQTT订阅数据 下行指令
#endif
            int startIndex = incomingByteItem.indexOf(flagMQTT);
            String start = incomingByteItem.substring(startIndex);
            int endIndex = start.indexOf("}"); //  发送JSON数据的换行 会导致后缀丢失 可尝试\n\r
            String end = start.substring(0, endIndex + 1);
            String data = end.substring(end.lastIndexOf("{"), end.length());
            vector<string> dataArray = split(start.c_str(), ",");
            String topic = dataArray[2].c_str();
            // String data = dataArray[3].c_str(); // JSON结构体可能有分隔符 导致分割不正确 可根据前一位集indexOf截取获取最后一位
#if IS_DEBUG
            Serial.printf("AT指令MQTT订阅主题: %s\n", topic.c_str());
            Serial.println(data);
#endif

            if (!data.isEmpty()) {
                DynamicJsonDocument json = string_to_json(data);
                // 获取MQTT订阅消息后执行任务
                do_at_mqtt_subscribe(json, topic);
            }
        }
        delay(10);
    }

    if (incomingByte.indexOf(flagCEREG) != -1) {  // 判断网络注册状态 参数1或5标识附着正常
        int startIndex = incomingByte.indexOf(flagCEREG);
        String start = incomingByte.substring(startIndex);
        int endIndex = start.indexOf("\n");
        String end = start.substring(0, endIndex + 1);
        String data = end.substring(0, end.length());
        vector<string> dataArray = split(data.c_str(), ",");
        int index = 0; // 取值下标
        if (data.indexOf(",") != -1) {
            index = 1;
        }
        String stat = dataArray[index].c_str();
        // Serial.println("网络注册状态 : " + data + " 状态值: " + stat);
        if (stat.toInt() == 1 || stat.toInt() == 5) {
            // 网络已连接 但不一定完全Ping通
        } else {
            // 网络不正常 无法连接
            Serial.println("NB-IoT 已断网触发重连机制...");
            hardware_restart_nb_iot(); // 硬件重启网络模组
            esp_restart();  // 重启单片机主控芯片
        }
    }

    if (incomingByte.indexOf(flagQENG) != -1) { // 模组工程模式 当前的网络服务信息 真正NB-IoT信号质量 走4G LTE部分带宽
        // 强：RSRP ≥ -100 dBm，且 SNR ≥ 3 dB
        // 中：-100 dBm ≥ RSRP ≥ -110 dBm 且 3 db > SNR > -3 db
        // 弱：RSRP < -115 dBm 或 SNR < -3 dB
        try {
            int startIndex = incomingByte.indexOf(flagQENG);
            String start = incomingByte.substring(startIndex);
            int endIndex = start.indexOf("\n");
            String end = start.substring(0, endIndex + 1);
            String data = end.substring(0, end.length());
            vector<string> dataArray = split(data.c_str(), ",");
            String RSRP = dataArray[5].c_str(); // 信号接收功率 关键参数
            String RSSI = dataArray[7].c_str();
            String SNR = dataArray[8].c_str();
            //Serial.println("RSRP: " + RSRP);
            //Serial.println("SNR: " + SNR);
            // NVS存储信号信息 用于MQTT上报
            String signal = "RSRP: " + RSRP + ",SNR: " + SNR + ",RSSI: " + RSSI;
            set_nvs("network_signal", signal.c_str());
            /*       if (RSRP.toInt() >= -100 && SNR.toInt() >= 3) {
                       // 信号强
                   } else if ((-110 <= RSRP.toInt() <= -100 && 3 < SNR.toInt() < 3) || RSRP.toInt() >= -110) {
                       // 信号中
                   } else if (RSRP.toInt() < -115 || SNR.toInt() < -3) {
                       // 信号弱
                   }*/
        } catch (exception &e) {
            cout << &e << endl;
        }
    }

/*    if (incomingByte.indexOf(flagCSQ) != -1) { // 2G/3G信号质量CSQ
        int startIndex = incomingByte.indexOf(flagCSQ);
        String start = incomingByte.substring(startIndex);
        int endIndex = start.indexOf("\n");
        String end = start.substring(0, endIndex + 1);
        String data = end.substring(0, end.length());
        // NVS存储信号信息 用于MQTT上报
        set_nvs("network_signal", data.c_str());

        vector<string> dataArray = split(data.c_str(), ",");
        String rssi = dataArray[0].c_str();
        if (rssi.c_str() == "+CSQ: 99" || rssi.c_str() == "+CSQ: 0" || rssi.c_str() == "+CSQ: 1") {
            Serial.println("NB-IoT CSQ信号强度丢失触发重连机制...");
            hardware_restart_nb_iot(); // 硬件重启网络模组
            esp_restart();  // 重启单片机主控芯片
        }
    }*/

    if (incomingByte.indexOf(flagCONN) != -1 || incomingByte.indexOf(flagSTAT) != -1) { // 检测MQTT连接状态断开
        // 检测MQTT服务状态 如果失效自动重连
        at_mqtt_reconnect(incomingByte);
    }

    // }
}

/**
 * 多线程MQTT任务
 */
void x_at_task_mqtt(void *pvParameters) {
    while (1) {
        // Serial.println("多线程MQTT任务, 心跳检测...");
        do_at_mqtt_heart_beat();
        delay(1000 * 600); // 多久执行一次 毫秒
    }
}

/**
 * 执行MQTT心跳
 */
void do_at_mqtt_heart_beat() {
    int deviceStatus = get_pwm_status(); // 设备电机状态
    int parkingStatus = ground_feeling_status(); // 是否有车
    String firmwareVersion = get_nvs("version"); // 固件版本
    String networkSignal = get_nvs("network_signal"); // 信号质量
    vector<string> array = split(to_string(get_electricity()), "."); // 电量值
    String electricityValue = array[0].c_str();
    // 发送心跳消息
    string jsonData =
            "{\"command\":\"heartbeat\",\"deviceCode\":\"" + to_string(get_chip_mac()) + "\",\"deviceStatus\":\"" +
            to_string(deviceStatus) + "\",\"parkingStatus\":\"" + to_string(parkingStatus) +
            "\",\"firmwareVersion\":\"" + firmwareVersion.c_str() + "\"," +
            "\"electricity\":\"" + electricityValue.c_str() + "\"," +
            "\"networkSignal\":\"" + networkSignal.c_str() + "\"}";
    at_mqtt_publish(at_topics, jsonData.c_str()); // 我是AT指令 MQTT心跳发的消息
}

/**
 * MQTT心跳服务
 */
void at_mqtt_heart_beat() {
#if !USE_MULTI_CORE
    const char *params = NULL;
    xTaskCreate(
            x_at_task_mqtt,  /* Task function. */
            "x_at_task_mqtt", /* String with name of task. */
            1024 * 8,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            8,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_at_task_mqtt, "x_at_task_mqtt", 8192, NULL, 8, NULL, 0);
#endif
}

/**
 * 获取MQTT订阅消息后执行任务
 */
void do_at_mqtt_subscribe(DynamicJsonDocument json, String topic) {
    // MQTT订阅消息处理 控制电机马达逻辑 可能重复下发指令使用QoS控制  并设置心跳检测
    String command = json["command"].as<String>();

#if IS_DEBUG
    Serial.println("指令类型: " + command);
#endif

    uint64_t chipId;
    try {
        chipId = get_chip_mac();
    } catch (exception &e) {
        cout << &e << endl;
    }

    // MQTT订阅消息处理
    if (topic.indexOf("ESP32/system") != -1) { // 针对主题做逻辑处理
        String chipIds = json["chipIds"].as<String>();  // 根据设备标识进行指定设备升级 为空全部升级 逗号分割
        vector<string> array = split(chipIds.c_str(), ",");
        bool isUpdateByDevice = false;
        if (std::find(array.begin(), array.end(), to_string(chipId)) != array.end()) {
            Serial.print("根据设备标识进行指定设备OTA升级: ");
            Serial.println(chipId);
            isUpdateByDevice = true;
        }

        if (command == "upgrade") { // MQTT通讯立刻执行OTA升级
            /*    {
                    "command": "upgrade",
                    "firmwareUrl" : "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/firmware.bin",
                     "chipIds" : ""
                } */
            String firmwareUrl = json["firmwareUrl"].as<String>();
            if (chipIds == "null" || chipIds.isEmpty() || isUpdateByDevice) {
                Serial.println("MQTT通讯立刻执行OTA升级");
                do_firmware_upgrade("", "", firmwareUrl); // 主动触发升级
            }
        } else if (command == "restart") { // 远程重启设备
            if (chipIds == "null" || chipIds.isEmpty() || isUpdateByDevice) {
                Serial.println("远程重启单片机设备...");
                esp_restart();
            }
        } else if (command == "restart_network") { // 远程重启网络
            if (chipIds == "null" || chipIds.isEmpty() || isUpdateByDevice) {
                Serial.println("远程重启单片机设备NB-IoT网络...");
                restart_nb_iot();
            }
        }
        return;
    }

    if (command == "heartbeat") { // 心跳指令
        do_at_mqtt_heart_beat();
    }
    if (command == "raise") { // 电机升起指令
        set_motor_up();
    }
    if (command == "putdown") { // 电机下降指令
        set_motor_down();
    }
    if (command == "query") { // MQTT主动查询指令
        int deviceStatus = get_pwm_status(); // 设备电机状态
        int parkingStatus = ground_feeling_status(); // 是否有车
        std:
        string jsonData = "{\"command\":\"query\",\"deviceCode\":\"" + to_string(chipId) + "\",\"deviceStatus\":\"" +
                          to_string(deviceStatus) + "\",\"parkingStatus\":\"" + to_string(parkingStatus) + "\"}";
        at_mqtt_publish(at_topics, jsonData.c_str());
    }
}

/**
 * MQTT订阅消息回调 中断机制
 */
void at_interrupt_mqtt_callback() {
    // 参考文章: https://lastminuteengineers.com/handling-esp32-gpio-interrupts-tutorial/
    // https://dreamsourcelab.cn/%e6%8a%80%e6%9c%af%e6%96%87%e7%ab%a0/%e6%9c%80%e8%af%a6%e7%bb%86%e7%9a%84-%e9%80%9a%e8%ae%af%e5%8d%8f%e8%ae%ae-uart%e5%88%86%e6%9e%90-%e5%9c%a8%e8%bf%99%e9%87%8c/
    // 使用外部中断机制 外设发出的中断请求 您无需不断检查引脚的当前值。使用中断，当检测到更改时，会触发事件（调用函数) 无需循环检测。 持续监控某种事件、时效性和资源使用情况更好
    // 将中断触发引脚 设置为INPUT_PULLUP（输入上拉）模式
    pinMode(0, INPUT_PULLUP);
    // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
    // LOW：当针脚输入为低时，触发中断。
    // HIGH：当针脚输入为高时，触发中断。
    // CHANGE：当针脚输入发生改变时，触发中断。
    // RISING：当针脚输入由低变高时，触发中断。
    // FALLING：当针脚输入由高变低时，触发中断。
/*  中断服务程序 (ISR) 是每次在 GPIO 引脚上发生中断时调用的函数
    // 1. ISR 不能有任何参数，它们不应该返回任何东西 2. ISR 应该尽可能短和快，因为它们会阻止正常的程序执行 3. IRAM_ATTR 编译后的代码被放置在单片机的内部 RAM (IRAM)中, 因Flash 比内部 RAM 慢得多
    void IRAM_ATTR ISR() {
        Statements;
    } */
    // attachInterrupt(0, at_mqtt_callback, FALLING);
}

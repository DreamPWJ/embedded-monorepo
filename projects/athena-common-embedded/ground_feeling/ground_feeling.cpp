#include "ground_feeling.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "at_mqtt/at_mqtt.h"
#include "chip_info.h"
#include <iostream>
#include <string>

using namespace std;

/**
* @author 潘维吉
* @date 2022/8/24 17:18
* @description 地磁传感器 如QMC5883三轴磁阻传感器
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

// 地感信号GPIO 外部中断接收
const int GROUND_FEELING_GPIO = 7;
const int GROUND_FEELING_RST_GPIO = 9;
const int GROUND_FEELING_CTRL_I_GPIO = 5;

// MQTT通用的Topic
const char *topic = "ESP32/common";

/**
 * 地磁信号GPIO外部中断
 */
void IRAM_ATTR check_has_car() {
    Serial.println("地磁检测有车, 进入外部中断了");
/*    // 芯片唯一标识
    uint64_t chipId = get_chip_mac();
    // 车辆驶入
    string jsonData =
            "{\"command\":\"parkingstatus\",\"msg\":\"车辆驶入了\",\"deviceCode\":\"" + to_string(chipId) +
            "\",\"parkingStatus\":\"" + to_string(1) +
            "\"}";
    at_mqtt_publish(topic, jsonData.c_str());*/
}

/**
 * 地磁信号GPIO外部中断
 */
void IRAM_ATTR check_no_car() {
    Serial.println("地磁检测无车, 进入外部中断了");
/*    // 芯片唯一标识
    uint64_t chipId = get_chip_mac();
    // 车辆驶出
    string jsonData =
            "{\"command\":\"parkingstatus\",\"msg\":\"车辆驶出了\",\"deviceCode\":\"" + to_string(chipId) +
            "\",\"parkingStatus\":\"" + to_string(0) +
            "\"}";
    at_mqtt_publish(topic, jsonData.c_str());*/
}

/**
 * 初始化地感信号GPIO
 */
void init_ground_feeling() {
    // GPIO接口使用前，必须初始化，设定引脚用于输入还是输出
    pinMode(GROUND_FEELING_GPIO, INPUT_PULLUP);
    pinMode(GROUND_FEELING_RST_GPIO, OUTPUT);
    pinMode(GROUND_FEELING_CTRL_I_GPIO, OUTPUT);
    // LOW：当针脚输入为低时，触发中断。
    // HIGH：当针脚输入为高时，触发中断。
    // CHANGE：当针脚输入发生改变时，触发中断。
    // RISING：当针脚输入由低变高时，触发中断。
    // FALLING：当针脚输入由高变低时，触发中断。
    //attachInterrupt(digitalPinToInterrupt(GROUND_FEELING_GPIO), check_has_car, HIGH); // 高电平表示检测到进车
    //attachInterrupt(digitalPinToInterrupt(GROUND_FEELING_GPIO), check_no_car, LOW);  // 低电平表示检测到出车

    digitalWrite(GROUND_FEELING_RST_GPIO, LOW);
    delay(500);
    digitalWrite(GROUND_FEELING_RST_GPIO, HIGH);
    delay(10);
    digitalWrite(GROUND_FEELING_CTRL_I_GPIO,HIGH);
    delay(10);
    Serial.print("MAG_OPEN\n"); // 三轴地磁传感器初始化 开始检测
    delay(1000);
    digitalWrite(GROUND_FEELING_CTRL_I_GPIO,LOW);
}

/**
 * 地磁信号检测
 */
int ground_feeling_status() {
    int ground_feeling = digitalRead(GROUND_FEELING_GPIO);
    // printf("GPIO %d 电平信号值: %d \n", GROUND_FEELING_GPIO, ground_feeling);
    if (ground_feeling == 1) {
        // printf("地感检测有车 \n");
        return 1;
    } else if (ground_feeling == 0) {
        // 如果无车时间超过一定时长  地锁抬起
        // printf("地感检测无车 \n");
        return 0;
    }
    return -1;
}

/**
 * 检测地感状态 有车实时上报MQTT服务器
 */
int lastTimeStatus; // 上次地感状态
void x_task_ground_feeling_status(void *pvParameters) {
    uint64_t chipId = get_chip_mac();

    while (1) {  // RTOS多任务条件： 1. 不断循环 2. 无return关键字
        // 确保上次检测是无车, 本次检测有车才上报 已上报不再上报
        int status = ground_feeling_status();
        if (lastTimeStatus == 0 && status == 1) {
            // 车辆驶入
            string jsonData =
                    "{\"command\":\"parkingstatus\",\"msg\":\"车辆驶入了\",\"deviceCode\":\"" + to_string(chipId) +
                    "\",\"parkingStatus\":\"" + to_string(status) +
                    "\"}";
            at_mqtt_publish(topic, jsonData.c_str());
        } else if (lastTimeStatus == 1 && status == 0) {
            // 车辆驶出
            string jsonData =
                    "{\"command\":\"parkingstatus\",\"msg\":\"车辆驶出了\",\"deviceCode\":\"" + to_string(chipId) +
                    "\",\"parkingStatus\":\"" + to_string(status) +
                    "\"}";
            at_mqtt_publish(topic, jsonData.c_str());
        }
        lastTimeStatus = status;
        delay(6000); // 多久执行一次 毫秒
    }
}

/**
 * 检测地感状态 有车实时上报MQTT服务器
 */
void check_ground_feeling_status() {
#if !USE_MULTI_CORE
    const char *params = NULL;
    xTaskCreate(
            x_task_ground_feeling_status,  /* Task function. */
            "x_task_ground_feeling_status", /* String with name of task. */
            1024 * 2,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            6,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_ground_feeling_status, "x_task_ground_feeling_status", 8192, NULL, 6, NULL, 0);
#endif
}
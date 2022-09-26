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
* @description 地感信号
*/

#define USE_MULTI_CORE 0 // 是否使用多核 根据芯片决定

// 地感信号GPIO
const int ground_feeling_gpio = 8;

/**
 * 初始化地感信号GPIO
 */
void init_ground_feeling() {
    // GPIO接口使用前，必须初始化，设定引脚用于输入还是输出
    pinMode(ground_feeling_gpio, INPUT_PULLUP);
}

/**
 * 地感信号检测
 */
int ground_feeling_status() {
    int ground_feeling = digitalRead(ground_feeling_gpio);
    // printf("GPIO %d 电平信号值: %d \n", ground_feeling_gpio, ground_feeling);
    if (ground_feeling == 0) {
        // printf("地感检测有车 \n");
        return 1;
    } else if (ground_feeling == 1) {
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
    uint32_t chipId = get_chip_mac();
    while (1) {  // RTOS多任务条件： 1. 不断循环 2. 无return关键字
        // 确保上次检测是无车, 本次检测有车才上报 已上报不再上报
        int status = ground_feeling_status();
        const char *topic = "ESP32/common";
        if (lastTimeStatus == 0 && status == 1) {
            // 车辆驶入
            string jsonData =
                    "{\"command\":\"parkingstatus\",\"msg\":\"车辆驶入了\",\"deviceCode\":\"\"" + to_string(chipId) +
                    "\",\"parkingStatus\":\"" + to_string(status) +
                    "\"}";
            at_mqtt_publish(topic, jsonData.c_str());
        } else if (lastTimeStatus == 1 && status == 0) {
            // 车辆驶出
            string jsonData =
                    "{\"command\":\"parkingstatus\",\"msg\":\"车辆驶出了\",\"deviceCode\":\"\"" + to_string(chipId) +
                    "\",\"parkingStatus\":\"" + to_string(status) +
                    "\"}";
            at_mqtt_publish(topic, jsonData.c_str());
        }
        lastTimeStatus = status;
        delay(5000); // 多久执行一次 毫秒
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
            8192,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            6,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_ground_feeling_status, "x_task_ground_feeling_status", 8192, NULL, 6, NULL, 0);
#endif
}
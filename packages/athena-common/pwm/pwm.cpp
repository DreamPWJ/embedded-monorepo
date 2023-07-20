#include "pwm.h"
#include <Arduino.h>
#include <ground_feeling.h>
#include <at_mqtt/at_mqtt.h>
#include <chip_info.h>
#include <iostream>
#include <string>

using namespace std;

/**
* @author 潘维吉
* @date 2022/8/24 10:21
* @description PWM脉冲宽度调是一种模拟控制方式 是将模拟信号转换为脉波的一种技术
*/

#define USE_MULTI_CORE 1 // 是否使用多核 根据芯片决定
#define IS_DEBUG false   // 是否调试模式

// 电机上下限位信号GPIO
const int MOTOR_UPPER_GPIO = 1; // 上限位
const int MOTOR_LOWER_GPIO = 2; // 下限位

// PWM控制引脚GPIO
const int PWM_PinA = 42;
const int PWM_PinB = 41;

// PWM的通道，共16个(0-15)，分为高低速两组，
// 高速通道(0-7): 80MHz时钟，低速通道(8-15): 1MHz时钟
// 0-15都可以设置，只要不重复即可，参考上面的列表
// 如果有定时器的使用，千万要避开!!!
const int channel_PWMA = 2;
const int channel_PWMB = 3;

// PWM波形频率KHZ
int freq_PWM = 5000;

// PWM占空比的分辨率，控制精度，取值为 0-20 之间
// 填写的pwm值就在 0 - 2的10次方 之间 也就是 0-1024
int resolution_PWM = 10;

const int GROUND_FEELING_RST_GPIO = 15;

const char *common_topic = "ESP32/common";
uint64_t chipMacId = get_chip_mac();

/**
 * 初始化PWM电机马达
 */
void init_motor() {
    Serial.println("初始化PWM电机马达");
    // GPIO接口使用前，必须初始化，设定引脚用于输入还是输出
    pinMode(MOTOR_UPPER_GPIO, INPUT_PULLUP);
    pinMode(MOTOR_LOWER_GPIO, INPUT_PULLUP);

    ledcSetup(channel_PWMA, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinA, channel_PWMA); // 将 LEDC 通道绑定到指定 IO 口上以实现输出
    ledcSetup(channel_PWMB, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinB, channel_PWMB);
}

/**
 * 控制电机马达抬起
 */
int channel_PWMA_duty;

void set_motor_up(int delay_time) {
#if IS_DEBUG
    // 上报MQTT消息
    string jsonData = "{\"msg\":\"开始控制电机正向运动\",\"chipId\":\"" + to_string(chipMacId) + "\"}";
    at_mqtt_publish(common_topic, jsonData.c_str());
#endif

    // 地感保证无车才能抬杆
    if (ground_feeling_status() == 1) {
        Serial.println("地磁判断有车地锁不能抬起");
        string jsonDataGF =
                "{\"command\":\"exception\",\"msg\":\"地磁判断有车地锁不能抬起\",\"chipId\":\"" + to_string(chipMacId) +
                "\"}";
        at_mqtt_publish(common_topic, jsonDataGF.c_str());
        return;
    }
    if (get_pwm_status() == 1) { // 如果已经在上限位 不触发电机
        return;
    }

    channel_PWMA_duty = 1024; // PWM速度值
    int overtime = 12; // 超时时间 秒s

    Serial.println("开始控制电机正向运动");
    stop_down_motor(); // 停止反向电机

    time_t startA = 0, endA = 0;
    double costA; // 时间差 秒
    time(&startA);
    ledcWrite(channel_PWMA, channel_PWMA_duty);
    // 读取限位信号 停机电机 同时超时后自动复位或停止电机
    delay(delay_time);
    while (get_pwm_status() == 2 && channel_PWMA_duty != 0) { // 在运动状态或PWM速度非0停止状态
        delay(10);
        time(&endA);
        costA = difftime(endA, startA);
        //printf("电机正向执行耗时：%f \n", costA);
        if (ground_feeling_status() == 1) {
            ledcWrite(channel_PWMA, 0); // 停止电机
            Serial.println("地磁判断有车地锁不能继续升起, 回落地锁");
            set_motor_down(); // 降锁
            break;
        }
        if (costA >= 1) { // 电机运行过半减速
            ledcWrite(channel_PWMB, 0);
            ledcWrite(channel_PWMA, channel_PWMA_duty);
            if (channel_PWMA_duty > 512) {
                channel_PWMA_duty = channel_PWMA_duty - 3;
            }
        }
        if (costA >= overtime) {
            printf("电机正向运行超时了 \n");
            string jsonDataUP =
                    "{\"command\":\"exception\",\"code\":\"1001\",\"msg\":\"车位锁电机抬起运行超时了\",\"chipId\":\"" +
                    to_string(chipMacId) + "\"}";
            at_mqtt_publish(common_topic, jsonDataUP.c_str());
            // ledcWrite(channel_PWMA, 0); // 停止电机
            Serial.println("电机正向运行超时不能继续升起, 回落地锁");
            set_motor_down(); // 降锁
            break;
        }
    }

    if (get_pwm_status() == 1) { // 如果已经在上限位
        Serial.println("检测到电机上限位触发");
        ledcWrite(channel_PWMA, 0); // 停止电机
        delay(200);
        digitalWrite(GROUND_FEELING_RST_GPIO, HIGH); // 关闭地感检测
    }

    ledcWrite(channel_PWMA, 0); // 停止电机

}

/**
 * 控制电机马达落下
 */
int channel_PWMB_duty;

void set_motor_down(int delay_time) {
#if IS_DEBUG
    // 上报MQTT消息
    string jsonData = "{\"msg\":\"开始控制电机反向运动\",\"chipId\":\"" + to_string(chipMacId) + "\"}";
    at_mqtt_publish(common_topic, jsonData.c_str());
#endif

    if (get_pwm_status() == 0) { // 如果已经在下限位 不触发电机
        return;
    }

    channel_PWMB_duty = 1024; // PWM速度值
    int overtime = 12; // 超时时间 秒s

    Serial.println("开始控制电机反向运动");
    stop_up_motor(); // 停止正向电机

    time_t startB = 0, endB = 0;
    double costB; // 时间差 秒
    time(&startB);
    ledcWrite(channel_PWMB, channel_PWMB_duty);
    delay(delay_time);
    digitalWrite(GROUND_FEELING_RST_GPIO, LOW); // 开启地感检测
    while (get_pwm_status() == 2 && channel_PWMB_duty != 0) {  // 在运动状态与PWM速度非0停止状态
        delay(10);
        time(&endB);
        costB = difftime(endB, startB);
        // printf("电机反向执行耗时：%f \n", costB);
        if (costB >= 1) { // 电机运行过半减速
            ledcWrite(channel_PWMA, 0);
            ledcWrite(channel_PWMB, channel_PWMB_duty);
            if (channel_PWMB_duty > 512) {
                channel_PWMB_duty = channel_PWMB_duty - 3;
            }
        }
        if (costB >= overtime) {
            printf("电机反向运行超时了 \n");
            string jsonDataDown =
                    "{\"command\":\"exception\",\"code\":\"1002\",\"msg\":\"车位锁电机降落运行超时了\",\"chipId\":\"" +
                    to_string(chipMacId) + "\"}";
            at_mqtt_publish(common_topic, jsonDataDown.c_str());
            ledcWrite(channel_PWMB, 0); // 停止电机
            break;
        }
    }

    if (get_pwm_status() == 0) { // 已经在下限位
        ledcWrite(channel_PWMB, 0); // 停止电机
        // MQTT上报已落锁完成 可用于灯控或语音提醒等
        string jsonDataDown =
                "{\"command\":\"lock_status\",\"msg\":\"车位锁已落锁完成\",\"deviceCode\":\"" + to_string(chipMacId) +
                "\",\"deviceStatus\":\"" + to_string(0) +
                "\"}";
        at_mqtt_publish(common_topic, jsonDataDown.c_str());
    }

    ledcWrite(channel_PWMB, 0); // 停止电机
}

/**
 * 停止电机
 */
void stop_motor() {
    ledcWrite(channel_PWMA, 0);
    ledcWrite(channel_PWMB, 0);
}

/**
 * 停止正向电机
 */
void stop_up_motor() {
    channel_PWMA_duty = 0;
    ledcWrite(channel_PWMA, channel_PWMA_duty);
}

/**
 * 停止反向电机
 */
void stop_down_motor() {
    channel_PWMB_duty = 0;
    ledcWrite(channel_PWMB, channel_PWMB_duty);
}

/**
 * 电机马达运作
 */
void set_pwm() {
    set_motor_up();
    delay(2000);
    set_motor_down();
}

/**
 * 电机马达运作状态检测
 */
int get_pwm_status() {
    // 读取后电平为0/1  中断机制
    int upper_limit = digitalRead(MOTOR_UPPER_GPIO);
    int lower_limit = digitalRead(MOTOR_LOWER_GPIO);
    // printf("GPIO %d 电平信号值: %d \n", MOTOR_UPPER_GPIO, upper_limit);
    // printf("GPIO %d 电平信号值: %d \n", MOTOR_LOWER_GPIO, lower_limit);
    if (upper_limit == 0 && lower_limit == 0) {
        //Serial.println("电机上限位状态触发");
        ledcWrite(channel_PWMA, 0);
        return 1;
    } else if (upper_limit == 1 && lower_limit == 1) {
        //Serial.println("电机下限位状态触发");
        ledcWrite(channel_PWMB, 0);
        return 0;
    } else if (upper_limit == 0 && lower_limit == 1) {
        //Serial.println("电机运行状态触发");
        return 2;
    } else if (upper_limit == 1 && lower_limit == 0) {
        //Serial.println("电机无效状态触发");
        return -1;
    }
    return -1;
}

void pwm_set_duty(uint16_t DutyA, uint16_t DutyB) {
    ledcWrite(channel_PWMA, DutyA);
    delay(1000);
    ledcWrite(channel_PWMB, DutyB);
}

/**
 * 检测电机状态
 */
void x_task_pwm_status(void *pvParameters) {
    while (1) {  // RTOS多任务条件： 1. 不断循环 2. 无return关键字
        delay(30 * 1000); // 多久执行一次 毫秒
        if (get_pwm_status() == -1) { // 无效状态
            Serial.println("电机无效状态触发, 复位中");
            int channel_duty = 1024; // PWM速度值
            ledcWrite(channel_PWMB, channel_duty);
            while (get_pwm_status() == -1 && channel_duty != 0) { // 在运动状态或PWM速度非0停止状态
                delay(10);
                ledcWrite(channel_PWMB, channel_duty);
                if (channel_duty > 512) {
                    channel_duty = channel_duty - 3;
                }
            }
            if (get_pwm_status() == 1) { // 如果已经在上限位
                Serial.println("复位检测到电机上限位触发");
                ledcWrite(channel_PWMB, 0); // 停止电机
            }
        }
    }
}

/**
 * 检测电机状态任务
 */
void check_pwm_status() {
#if !USE_MULTI_CORE
    const char *params = NULL;
    xTaskCreate(
            x_task_pwm_status,  /* Task function. */
            "x_task_pwm_status", /* String with name of task. */
            1024 * 2,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            6,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
#else
    // 最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1, 或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(x_task_pwm_status, "x_task_pwm_status",
                            1024 * 2, NULL, 6, NULL, 0);
#endif
}
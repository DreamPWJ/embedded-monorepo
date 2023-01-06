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

#define IS_DEBUG false  // 是否调试模式

// PWM控制引脚GPIO
const int PWM_PinA = 3;
const int PWM_PinB = 2;
// 电机驱动模块控制信号
/*const int Motor_INA1 = 2;
const int Motor_INA2 = 3;*/
/*const int Motor_INB1 = 17;
const int Motor_INB2 = 18;*/

// PWM的通道，共16个(0-15)，分为高低速两组，
// 高速通道(0-7): 80MHz时钟，低速通道(8-15): 1MHz时钟
// 0-15都可以设置，只要不重复即可，参考上面的列表
// 如果有定时器的使用，千万要避开!!!
const int channel_PWMA = 2;
const int channel_PWMB = 3;

// 电机上下限信号GPIO
const int motor_lower_limit = 0; // 下限位
const int motor_upper_limit = 1; // 上限位

// PWM波形频率KHZ
int freq_PWM = 5000;

// PWM占空比的分辨率，控制精度，取值为 0-20 之间
// 填写的pwm值就在 0 - 2的10次方 之间 也就是 0-1024
int resolution_PWM = 10;

const int GROUND_FEELING_RST_GPIO = 9;
const int GROUND_FEELING_CTRL_I_GPIO = 5;
const char *common_topic = "ESP32/common";
uint64_t chipMacId = get_chip_mac();

/**
 * 初始化PWM电机马达
 */
void init_motor() {
    Serial.println("初始化PWM电机马达");
    // GPIO接口使用前，必须初始化，设定引脚用于输入还是输出
    pinMode(motor_upper_limit, INPUT_PULLUP);
    pinMode(motor_lower_limit, INPUT_PULLUP);
/*    pinMode(Motor_INA1, OUTPUT);
      pinMode(Motor_INA2, OUTPUT); */
    /*  pinMode(Motor_INB1, OUTPUT);
      pinMode(Motor_INB2, OUTPUT); */

    ledcSetup(channel_PWMA, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinA, channel_PWMA); // 将 LEDC 通道绑定到指定 IO 口上以实现输出
    ledcSetup(channel_PWMB, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinB, channel_PWMB);
}

/**
 * 控制电机马达抬起
 */
int channel_PWMA_duty;

void set_motor_up() {
#if IS_DEBUG
    // 上报MQTT消息
    string jsonData = "{\"msg\":\"开始控制电机正向运动\",\"chipId\":\"" + to_string(chipMacId) + "\"}";
    at_mqtt_publish(common_topic, jsonData.c_str());
#endif

    // 地感保证无车才能抬杆
    if (ground_feeling_status() == 1) {
        Serial.println("地感判断有车地锁不能抬起");
        string jsonDataGF =
                "{\"command\":\"exception\",\"msg\":\"地感判断有车地锁不能抬起\",\"chipId\":\"" + to_string(chipMacId) +
                "\"}";
        at_mqtt_publish(common_topic, jsonDataGF.c_str());
        return;
    }
    if (get_pwm_status() == 1) { // 如果已经在上限位 不触发电机
        return;
    }

    channel_PWMA_duty = 1024; // PWM速度值
    int overtime = 10; // 超时时间 秒s

    Serial.println("开始控制电机正向运动");
    stop_down_motor(); // 停止反向电机

    time_t startA = 0, endA = 0;
    double costA; // 时间差 秒
    time(&startA);
    ledcWrite(channel_PWMA, channel_PWMA_duty);
    // 读取限位信号 停机电机 同时超时后自动复位或停止电机
    delay(600);
    if (get_pwm_status() == 2) {
        digitalWrite(GROUND_FEELING_CTRL_I_GPIO, HIGH);
        Serial.println("MAG_STOP\n"); // 升锁同时停止地磁检测
    }
    while (get_pwm_status() == 2 && channel_PWMA_duty != 0) { // 在运动状态或PWM速度非0停止状态
        delay(10);
        time(&endA);
        costA = difftime(endA, startA);
        //printf("电机正向执行耗时：%f \n", costA);
        if (ground_feeling_status() == 1) {
            ledcWrite(channel_PWMA, 0); // 停止电机
            Serial.println("地感判断有车地锁不能继续抬起, 回落地锁");
            set_motor_down(); // 回落锁
            digitalWrite(GROUND_FEELING_CTRL_I_GPIO, HIGH);
            Serial.println("MAG_CONT\n"); // 若升锁遇阻，说明模块检测出错，主控应再次落锁
            break;
        }
        if (costA >= 3) { // 电机运行过半减速
            ledcWrite(channel_PWMB, 0);
            ledcWrite(channel_PWMA, channel_PWMA_duty);
            channel_PWMA_duty = channel_PWMA_duty - 2;
        }
        if (costA >= overtime) {
            printf("电机正向运行超时了 \n");
            string jsonDataUP =
                    "{\"command\":\"exception\",\"code\":\"1001\",\"msg\":\"车位锁电机抬起运行超时了\",\"chipId\":\"" +
                    to_string(chipMacId) + "\"}";
            at_mqtt_publish(common_topic, jsonDataUP.c_str());
            ledcWrite(channel_PWMA, 0); // 停止电机
            break;
        }
    }
}

/**
 * 控制电机马达落下
 */
int channel_PWMB_duty;

void set_motor_down() {
#if IS_DEBUG
    // 上报MQTT消息
    string jsonData = "{\"msg\":\"开始控制电机反向运动\",\"chipId\":\"" + to_string(chipMacId) + "\"}";
    at_mqtt_publish(common_topic, jsonData.c_str());
#endif

    if (get_pwm_status() == 0) { // 如果已经在下限位 不触发电机
        return;
    }

    channel_PWMB_duty = 1024; // PWM速度值
    int overtime = 10; // 超时时间 秒s

    Serial.println("开始控制电机反向运动");
    stop_up_motor(); // 停止正向电机

    time_t startB = 0, endB = 0;
    double costB; // 时间差 秒
    time(&startB);
    ledcWrite(channel_PWMB, channel_PWMB_duty);
    delay(600);
    while (get_pwm_status() == 2 && channel_PWMB_duty != 0) {  // 在运动状态与PWM速度非0停止状态
        delay(10);
        time(&endB);
        costB = difftime(endB, startB);
        // printf("电机反向执行耗时：%f \n", costB);
        if (costB >= 3) { // 电机运行过半减速
            ledcWrite(channel_PWMA, 0);
            ledcWrite(channel_PWMB, channel_PWMB_duty);
            channel_PWMB_duty = channel_PWMB_duty - 2;
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

    digitalWrite(GROUND_FEELING_RST_GPIO, LOW);
    delay(1500);
    digitalWrite(GROUND_FEELING_RST_GPIO, HIGH);
    digitalWrite(GROUND_FEELING_CTRL_I_GPIO, HIGH);
    Serial.println("MAG_OPEN\n"); // 落锁后开始地磁检测
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
    int upper_limit = digitalRead(motor_upper_limit);
    int lower_limit = digitalRead(motor_lower_limit);
    // printf("GPIO %d 电平信号值: %d \n", motor_upper_limit, upper_limit);
    // printf("GPIO %d 电平信号值: %d \n", motor_lower_limit, lower_limit);
    if (upper_limit == 0 && lower_limit == 1) {
        ledcWrite(channel_PWMA, 0);
        //Serial.println("电机上限位状态触发");
        return 1;
    } else if (upper_limit == 1 && lower_limit == 0) {
        ledcWrite(channel_PWMB, 0);
        //Serial.println("电机下限位状态触发");
        return 0;
    } else if (upper_limit == 1 && lower_limit == 1) {
        //Serial.println("电机运行状态触发");
        return 2;
    } else if (upper_limit == 0 && lower_limit == 0) {
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

// 电机的控制程序，分别是左右两个轮子的占空比（0-1024）
/*void motor_control(int Cnt_L, int Cnt_R)
{
    if (Cnt_L >= 0) // 左轮正向转
    {
        digitalWrite(Motor_INA1, HIGH);
        digitalWrite(Motor_INA2, LOW);
        ledcWrite(channel_PWMA, Cnt_L);
    }
    else // 左轮反向转
    {
        digitalWrite(Motor_INA1, LOW);
        digitalWrite(Motor_INA2, HIGH);
        ledcWrite(channel_PWMA, -Cnt_L);
    }

    if (Cnt_R >= 0) // 右轮正向转
    {
        digitalWrite(Motor_INB1, HIGH);
        digitalWrite(Motor_INB2, LOW);
        ledcWrite(channel_PWMB, Cnt_R);
    }
    else // 右轮反向转
    {
        digitalWrite(Motor_INB1, LOW);
        digitalWrite(Motor_INB2, HIGH);
        ledcWrite(channel_PWMB, -Cnt_R);
    }
}*/

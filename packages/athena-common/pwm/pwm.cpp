#include "pwm.h"
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/8/24 10:21
* @description PWM脉冲宽度调是一种模拟控制方式 是将模拟信号转换为脉波的一种技术
*/

/*
* LEDC Chan to Group/Channel/Timer Mapping
** ledc: 0  => Group: 0, Channel: 0, Timer: 0
** ledc: 1  => Group: 0, Channel: 1, Timer: 0
** ledc: 2  => Group: 0, Channel: 2, Timer: 1
** ledc: 3  => Group: 0, Channel: 3, Timer: 1
** ledc: 4  => Group: 0, Channel: 4, Timer: 2
** ledc: 5  => Group: 0, Channel: 5, Timer: 2
** ledc: 6  => Group: 0, Channel: 6, Timer: 3
** ledc: 7  => Group: 0, Channel: 7, Timer: 3
** ledc: 8  => Group: 1, Channel: 0, Timer: 0
** ledc: 9  => Group: 1, Channel: 1, Timer: 0
** ledc: 10 => Group: 1, Channel: 2, Timer: 1
** ledc: 11 => Group: 1, Channel: 3, Timer: 1
** ledc: 12 => Group: 1, Channel: 4, Timer: 2
** ledc: 13 => Group: 1, Channel: 5, Timer: 2
** ledc: 14 => Group: 1, Channel: 6, Timer: 3
** ledc: 15 => Group: 1, Channel: 7, Timer: 3
*/


// PWM控制引脚GPIO
const int PWM_PinA = 4;
const int PWM_PinB = 5;
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

/**
 * 初始化PWM电机马达
 */
void init_motor() {
    Serial.println("初始化PWM电机马达");
    // GPIO接口使用前，必须初始化，设定引脚用于输入还是输出
    pinMode(motor_upper_limit, INPUT_PULLUP);
    pinMode(motor_lower_limit, INPUT_PULLUP);
/*    pinMode(Motor_INA1, OUTPUT);
    pinMode(Motor_INA2, OUTPUT);*/
    /*  pinMode(Motor_INB1, OUTPUT);
      pinMode(Motor_INB2, OUTPUT); */

    ledcSetup(channel_PWMA, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinA, channel_PWMA); // 将 LEDC 通道绑定到指定 IO 口上以实现输出
    ledcSetup(channel_PWMB, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinB, channel_PWMB);
}

/**
 * 电机马达运作
 */
void set_pwm() {
    int overtime = 10;// 超时时间 秒s

    Serial.println("开始控制电机正向");
    time_t startA = 0, endA = 0;
    double costA; // 时间差 秒
    time(&startA);
    int channel_PWMA_duty = 1024;
    ledcWrite(channel_PWMA, channel_PWMA_duty);
    ledcWrite(channel_PWMB, 0);
    // 读取限位信号 停机电机 同时超时后自动复位或停止电机
    delay(1000);
    while (get_pwm_status() == 2) {
        delay(10);
        time(&endA);
        costA = difftime(endA, startA);
        //printf("电机正向执行耗时：%f \n", costA);
        if (costA >= 2) { // 电机运行过半减速
            ledcWrite(channel_PWMA, channel_PWMA_duty);
            ledcWrite(channel_PWMB, 0);
            channel_PWMA_duty = channel_PWMA_duty - 1;
        }
        if (costA >= overtime) {
            printf("电机正向运行超时了 \n");
            ledcWrite(channel_PWMA, 0); // 停止电机
            break;
        }
    }

    delay(2000);

    Serial.println("开始控制电机反向");
    time_t startB = 0, endB = 0;
    double costB; // 时间差 秒
    time(&startB);
    int channel_PWMB_duty = 1024;
    ledcWrite(channel_PWMB, channel_PWMB_duty);
    ledcWrite(channel_PWMA, 0);
    delay(1000);
    while (get_pwm_status() == 2) {
        delay(10);
        time(&endB);
        costB = difftime(endB, startB);
        // printf("电机反向执行耗时：%f \n", costB);
        if (costA >= 2) { // 电机运行过半减速
            ledcWrite(channel_PWMB, channel_PWMB_duty);
            ledcWrite(channel_PWMA, 0);
            channel_PWMB_duty = channel_PWMB_duty - 1;
        }
        if (costB >= overtime) {
            printf("电机反向运行超时了 \n");
            ledcWrite(channel_PWMB, 0); // 停止电机
            break;
        }
    }

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
    // delay(1000);
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

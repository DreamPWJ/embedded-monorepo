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

// PWM波形频率KHZ
int freq_PWM = 50;

// PWM占空比的分辨率，控制精度，取值为 0-20 之间
// 填写的pwm值就在 0 - 2的10次方 之间 也就是 0-1024
int resolution_PWM = 10;

/**
 * 初始化PWM电机马达
 */
void init_motor() {
    Serial.println("初始化PWM电机马达");
/*    pinMode(Motor_INA1, OUTPUT);
    pinMode(Motor_INA2, OUTPUT);*/
    /*  pinMode(Motor_INB1, OUTPUT);
      pinMode(Motor_INB2, OUTPUT); */

    ledcSetup(channel_PWMA, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinA, channel_PWMA); // 将 LEDC 通道绑定到指定 IO 口上以实现输出
    ledcSetup(channel_PWMB, freq_PWM, resolution_PWM); // 设置通道
    ledcAttachPin(PWM_PinB, channel_PWMB);
}

void set_pwm() {

    Serial.println("开始启动控制电机A");
/*    digitalWrite(Motor_INA1, HIGH);
    digitalWrite(Motor_INA2, LOW);*/
    ledcWrite(channel_PWMA, 512);
    ledcWrite(channel_PWMB, 0);

    Serial.println("开始启动控制电机B");
    ledcWrite(channel_PWMB, 512);
    ledcWrite(channel_PWMA, 0);

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

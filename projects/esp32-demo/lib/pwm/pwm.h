#ifndef ESP32_DEMO_MQTT_H
#define ESP32_DEMO_MQTT_H

#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/8/20 15:51
* @description PWM是一种模拟控制方式 是将模拟信号转换为脉波的一种技术
*/

void init_motor();

void pwm_set_duty(uint16_t DutyA, uint16_t DutyB);
void motor_control(int Cnt_L, int Cnt_R);

#endif

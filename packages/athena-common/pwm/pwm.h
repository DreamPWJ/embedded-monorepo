#ifndef EMBEDDED_MONOREPO_PWM_H
#define EMBEDDED_MONOREPO_PWM_H
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/8/24 10:21
* @description PWM脉冲宽度调是一种模拟控制方式 是将模拟信号转换为脉波的一种技术
* 参考文档： https://github.com/DreamPWJ/ESP32_Code/blob/main/2.Code/2.ESP32_Timer_PWM/src/main.cpp
*/

void init_motor();

void init_simple_motor();

void set_motor_up(int delay_time = 800);

void set_motor_down(int delay_time = 800);

void stop_motor();

void stop_up_motor();

void stop_down_motor();

void set_pwm();

int get_pwm_status();

void pwm_set_duty(uint16_t DutyA, uint16_t DutyB);

void x_task_pwm_status(void *pvParameters);

void check_pwm_status();

void set_simple_motor_up();

#endif

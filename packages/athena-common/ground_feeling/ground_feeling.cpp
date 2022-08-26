#include "ground_feeling.h"
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/8/24 17:18
* @description 地感信号
*/

// 地感信号GPIO
const int ground_feeling_gpio = 9;

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
void ground_feeling_status() {
    int ground_feeling = digitalRead(ground_feeling_gpio);
    // printf("GPIO %d 电平信号值: %d \n", ground_feeling_gpio, ground_feeling);
    if (ground_feeling == 0) {
        printf("地感检测有车 \n");
    } else if (ground_feeling == 1) {
        // 如果无车时间超过一定时长  地锁抬起
        printf("地感检测无车 \n");
    }
}
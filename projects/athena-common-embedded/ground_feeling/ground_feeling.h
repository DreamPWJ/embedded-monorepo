#ifndef EMBEDDED_MONOREPO_GROUND_FEELING_H
#define EMBEDDED_MONOREPO_GROUND_FEELING_H
#include <Arduino.h>
/**
* @author 潘维吉
* @date 2022/8/24 17:18
* @description 地感信号
*/

void init_ground_feeling();

int ground_feeling_status();

void x_task_ground_feeling_status(void *pvParameters);

void check_ground_feeling_status();

#endif

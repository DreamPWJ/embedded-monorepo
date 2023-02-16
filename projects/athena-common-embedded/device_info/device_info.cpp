#include "device_info.h"
#include <Arduino.h>
#include "driver/temp_sensor.h"

/**
* @author 潘维吉
* @date 2022/9/5 13:05
* @description 设备信息  如温度、电量等
* 各种传感器库： https://github.com/adafruit
*/


/**
 * 初始化温度信息 ESP32自带芯片温度传感器  ESP32 S3在 arduino-esp32的2.0.5才支持
 */
void init_temperature() {
    temp_sensor_config_t temp_sensor = {
            .dac_offset = TSENS_DAC_L2,
            .clk_div = 6
    };
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
}

/**
 * 获取温度信息
 */
float get_temperature() {
    float temperature = temp_sensor_read_celsius(&temperature);
    return temperature;
}

/**
 * 获取电量信息
 * 参考： https://randomnerdtutorials.com/power-esp32-esp8266-solar-panels-battery-level-monitoring/
 */
float get_electricity() {
    int ADC_GPIO = 6; // 监控电池电量IO引脚 必须是模拟数字输入ADC
    float in_min = 724.0f; // 输出最小电压伏
    float in_max = 869.0f; // 输出最大电压伏
    // set the resolution bits (0-4096)
    // analogReadResolution(6); // 衰减值
    // pinMode(GPIO, ANALOG);
    // 模拟引脚读取电池的输出电压  需要添加分压器，以便我们能够读取电池的电压
    int analogValue = analogRead(ADC_GPIO);   // 获取模拟值
    //  int analogVolts = analogReadMilliVolts(ADC_GPIO);  // 获取毫伏电压
    // Serial.println(analogValue);
    // 获取电量百分比
/*    float batteryLevel = map(analogValue, in_min, in_max, 0, 100);
    if (batteryLevel >= 100) {
        return 100;
    } else {
        return batteryLevel;
    }*/
    return analogValue * (5.0 / 1023.0);
}
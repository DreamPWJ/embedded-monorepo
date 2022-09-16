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
 * 初始化温度信息 ESP32自带芯片温度传感器
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
    int GPIO = 4; // 监控电池电量IO引脚 必须是模拟输入ADC
    // pinMode(GPIO, ANALOG);
    // 模拟引脚读取电池的输出电压  需要添加分压器，以便我们能够读取电池的电压
    // 获取电量值
    int value = analogRead(GPIO);
    // 获取电量百分比
    float batteryLevel = map(value, 0.0f, 4095.0f, 0, 100);
    return batteryLevel;
}
#include "device_info.h"
#include "driver/temp_sensor.h"

/**
* @author 潘维吉
* @date 2022/9/5 13:05
* @description 设备信息  如温度、电量等
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
    float temperature;
    temp_sensor_read_celsius(&temperature);
    return temperature;
}

/**
 * 获取电量信息
 */
int get_electricity() {
    return 100;
}
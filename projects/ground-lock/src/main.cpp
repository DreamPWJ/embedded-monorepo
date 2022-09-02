#include <Arduino.h>
#include <led_pin.h>
#include <nb_iot.h>
#include <wifi_network.h>
#include <mqtt.h>
#include <pwm.h>
#include <ground_feeling.h>
#include <ota.h>
#include <chip_info.h>
#include <vector>
#include <iostream>
#include <common_utils.h>

using namespace std;


#define FIRMWARE_VERSION              "CI_OTA_FIRMWARE_VERSION"  // 版本号用于OTA升级和远程升级文件对比 判断是否有新版本 每次需要OTA的时候更改设置 CI_OTA_FIRMWARE_VERSION关键字用于CI替换版本号
#define FIRMWARE_UPDATE_JSON_URL      "http://archive-artifacts-pipeline.oss-cn-shanghai.aliyuncs.com/iot/ground-lock/prod/ground-lockota.json" // 如果https证书有问题 可以使用http协议
#define WIFI_EN 1 // 是否开启WIFI网络功能 0 关闭  1 开启
#define MQTT_EN 0 // 是否开启MQTT消息协议 0 关闭  1 开启
#define PWM_EN 1 // 是否开启PWM脉冲宽度调制功能 0 关闭  1 开启

String mqttName = "esp32-mcu-client"; // mqtt客户端名称

char *pcTaskName;

void xTaskOTA(void *pvParameters) {
  /*  pcTaskName = (char *) pvParameters;
    std::vector<string> res = split(pcTaskName, ",");
    Serial.println(res[0].c_str());
    Serial.println(res[1].c_str());*/
    while (1) {
        exec_ota(FIRMWARE_VERSION, FIRMWARE_UPDATE_JSON_URL);
        delay(60000);
    }
}

void setup() {
    // 初始化设置代码

    // 设置串口波特率
    Serial.begin(115200);

    // 将LED数字引脚初始化为输出
    set_pin_mode();

    // 初始化NB-IoT网络协议
    init_nb_iot();

#if WIFI_EN
    // 初始化WiFi无线网络
    init_wifi();
#endif

#if MQTT_EN
    // 初始化MQTT消息协议
    init_mqtt(mqttName);
#endif

#if PWM_EN
    // 初始化电机马达
    init_motor();
#endif

    // 初始化地感线圈
    init_ground_feeling();

    // 执行OTA空中升级
    // exec_ota(FIRMWARE_VERSION, FIRMWARE_UPDATE_JSON_URL);
    const char *params = "1.1.2,https://panweiji.com"; // 逗号分割多参数
    xTaskCreate(
            xTaskOTA,  /* Task function. */
            "TaskOTA", /* String with name of task. */
            8192,      /* Stack size in bytes. */
            (void *) params,      /* Parameter passed as input of the task */
            5,         /* Priority of the task.(configMAX_PRIORITIES - 1 being the highest, and 0 being the lowest.) */
            NULL);     /* Task handle. */
}

void loop() {
    // 循环执行代码

    // 开发板LED 闪动的实现
    set_led();

#if WIFI_EN
    // 定时检测重新连接WiFi
    reconnect_wifi();
#endif

#if PWM_EN
    // 驱动电机马达工作
    // set_pwm();
#endif

#if MQTT_EN
    // MQTT消息服务
    mqtt_reconnect(mqttName);
    mqtt_loop();
#endif

    // 地感状态检测  判断是否有车
    //ground_feeling_status();

}
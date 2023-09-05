#include <Arduino.h>
#include <wifi_network.h>
#include <mqtt.h>
#include <bluetooth_connect.h>
// #include <log_insight.h>

/**
* @author 潘维吉
* @date 2022/7/20 9:41
* @description 程序运行入口
*/

// 获取自定义多环境变量宏定义
#define XSTR(x) #x
#define STR(x) XSTR(x)

static const char *TAG = "esp32_demo";

void setup() {
// write your initialization code here
    // 初始化设置代码  为保证单片机运行正常  电路设计与电压必须稳定

    // 设置UART串口波特率
    Serial.begin(115200);

    Serial.println("ESP32 C3 MCU");
    const char *env_name = STR(ENV_NAME);
    Serial.println(env_name);
    if (env_name == "app1") {
        Serial.println("App1环境工程");
    } else if (env_name == "app2") {
        Serial.println("App2环境工程");
    }
    const char *env_app_version = STR(ENV_APP_VERSION);
    Serial.println(env_app_version);
    const char *app_version = STR(APP_VERSION);
    Serial.println(app_version);
    const char *mqtt_broker = STR(MQTT_BROKER);
    Serial.println(mqtt_broker);
    std::string const &ota_temp_json = std::string("http://") + std::string(STR(FIRMWARE_UPDATE_JSON_URL));
    const char *firmware_update_json_url = ota_temp_json.c_str();
    Serial.println(firmware_update_json_url);
    Serial.println(STR(CORE_DEBUG_LEVEL));

    // 初始化设置蓝牙
    // init_bluetooth("panweiji");
    // 初始化WiFi无线网络
    init_wifi();
    // WiFi网络版本初始化MQTT消息协议
    init_mqtt();

    delay(3000);
    // 初始化日志云上报
//    init_insights();
//    ESP_LOGI(TAG, "初始化insights日志云上报");
//    ESP_LOGE(TAG, "初始化insights日志云上报, 错误日志");

}

void loop() {
// write your code here

}
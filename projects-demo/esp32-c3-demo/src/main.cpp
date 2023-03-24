#include <Arduino.h>
#include <wifi_network.h>
#include <mqtt.h>
#include <bluetooth_connect.h>
#include <log_insight.h>

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
    const char *mqtt_version = STR(MQTT_VERSION);
    Serial.println(mqtt_version);
    const char *app_version = STR(APP_VERSION);
    Serial.println(app_version);
    std::string const &ota_temp_json = std::string("http://") + std::string(STR(FIRMWARE_UPDATE_JSON_URL));
    const char *firmware_update_json_url = ota_temp_json.c_str();
    Serial.println(firmware_update_json_url);
    Serial.println(STR(CORE_DEBUG_LEVEL));

    // 初始化设置蓝牙
    // init_bluetooth("panweiji");
    // 初始化WiFi无线网络
    init_wifi();
    // 初始化日志云上报
    init_insights();
    delay(3000);
    ESP_LOGI(TAG, "初始化insights日志云上报");
    ESP_LOGE(TAG, "初始化insights日志云上报, 错误日志");
    // WiFi网络版本初始化MQTT消息协议
    //init_mqtt();
}

void loop() {
// write your code here

}
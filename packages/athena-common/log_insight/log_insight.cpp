#include "log_insight.h"

#include <Arduino.h>
#include <esp_insights.h>


/**
* @author 潘维吉
* @date 2022/8/22 15:59
* @description ESP Insights：连接设备的远程状态日志诊断/可观察性框架
* 参考文档: https://github.com/espressif/esp-insights#getting-started
*/

#define ESP_INSIGHTS_AUTH_KEY "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiZDhhYWEwODQtNWQ4MS00MjVlLWJjZDEtNGUxYWMzNWIzY2M3IiwiZXhwIjoxOTc2NTE1NTk3LCJpYXQiOjE2NjExNTU1OTcsImlzcyI6ImUzMjJiNTljLTYzY2MtNGU0MC04ZWEyLTRlNzc2NjU0NWNjYSIsInN1YiI6IjMyNWY1ZWEyLTJhY2EtNGZiZC1iNDNlLTRkYjgzYTY3ZmMyZiJ9.iJHoBczjOqGRXxGcbqW9WG9U7d2W7nzygc5sDXv1TIg8p5aCG0nXrUQkU-R2U265qFqlbI5qfsxJ5hPwkjMGgusApdoK78Ypi-yN-AtMw1jMPaM74v29qDCgoNoCno7qR5sKCvI6S8AnMt5E6fSwI5CuFoFbv3NpZhDgNc1Dye8Vp2S0UT1S9rI5d8OvPWW5p4xhe6Ytdh3-Xqd-8rIfRmiK1x1dx7Cx3i8s3FV5PQu8u3RurdEit725uHRc12oglCQWrhm293Pjqsgkk_BDHzeX9CmtxFHKbasTDVcb9vhRn9no_zFyrHRjDyU_1whC0pgyrbOLbyswXt3gAec3OA"

/**
 * 初始化日志上报功能
 */
void init_insights() {
    esp_insights_config_t config = {
            .log_type = ESP_DIAG_LOG_TYPE_ERROR,
            .auth_key = ESP_INSIGHTS_AUTH_KEY,
    };

    esp_insights_init(&config);

    /* Rest of the application initialization */
}
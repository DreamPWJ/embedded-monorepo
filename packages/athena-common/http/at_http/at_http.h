#ifndef EMBEDDED_MONOREPO_AT_HTTP_H
#define EMBEDDED_MONOREPO_AT_HTTP_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
* @author 潘维吉
* @date 2022/9/14 9:03
* @description 基于AT指令的HTTP工具
*/

DynamicJsonDocument at_http_get(String url, bool isResponseData = true);

String send_http_at_command(String command, const int timeout, boolean isDebug, String successResult = "OK");

DynamicJsonDocument get_http_uart_data();

#endif

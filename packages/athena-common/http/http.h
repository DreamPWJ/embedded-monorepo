#ifndef EMBEDDED_MONOREPO_HTTP_H
#define EMBEDDED_MONOREPO_HTTP_H

#include <HTTPClient.h>
#include <WString.h>
#include <ArduinoJson.h>
/**
* @author 潘维吉
* @date 2022/7/20 14:52
* @description Http网络请求工具
*/

DynamicJsonDocument http_get(String url);

void http_post(String url, String data);

void esp_http();


#endif

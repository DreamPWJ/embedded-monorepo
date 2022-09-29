#ifndef EMBEDDED_MONOREPO_JSON_UTILS_H
#define EMBEDDED_MONOREPO_JSON_UTILS_H
#include <Arduino.h>
#include <ArduinoJson.h>

/**
* @author 潘维吉
* @date 2022/9/13 9:23
* @description JSON数据类型工具类
*/

DynamicJsonDocument string_to_json(String data);

String json_to_string(JsonObject data);

// DynamicJsonDocument read_json_file(String filePath);

#endif

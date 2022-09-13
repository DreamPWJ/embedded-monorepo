#ifndef EMBEDDED_MONOREPO_JSON_UTILS_H
#define EMBEDDED_MONOREPO_JSON_UTILS_H
#include <Arduino.h>

/**
* @author 潘维吉
* @date 2022/9/13 9:23
* @description JSON数据类型工具类
*/

String json_to_string(JsonObject data);

JsonObject string_to_json(String data);

#endif

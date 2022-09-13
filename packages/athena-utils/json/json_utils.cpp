#include "json_utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>

/**
* @author 潘维吉
* @date 2022/9/13 9:23
* @description JSON数据类型工具类
* 项目地址： https://github.com/bblanchon/ArduinoJson
*/


/**
* 将String格式转换成Json
*/
JsonObject string_to_json(String data) {
    DynamicJsonDocument doc(2048);
    // 用String类型的变量来代替串口获取的Json数据
    // String input = "{\"id\":\"l or r\",\"speed\":10.50, \"kp\":5.1, \"ki\":0.1, \"kd\":0.02, \"forward\":1}"
    deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();
    // 可以用 obj[键名] 提取数据
    //Serial.println(obj["id"]);
    // 如果是 { "PID":[5.1, 0.1, 0.02] }
    // 则可以 kp = obj["PID"][0], ki = obj["PID"][1], kd = obj["PID"][2]
    // 可以将他们存入变量
    //String id = obj["id"];
    return obj;
}

/**
* 将Json格式转换成String
*/
String json_to_string(JsonObject data) {
    DynamicJsonDocument doc(2048);
    JsonObject obj = doc.as<JsonObject>();
    // 将数据键值对赋值
    obj["name"] = "潘维吉";
    obj["age"] = 18;
    String jsonStr;
    serializeJson(doc, jsonStr);
    return jsonStr;
}

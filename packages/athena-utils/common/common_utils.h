#ifndef EMBEDDED_MONOREPO_COMMON_UTILS_H
#define EMBEDDED_MONOREPO_COMMON_UTILS_H

#include <Arduino.h>
#include <vector>

using namespace std;

/**
* @author 潘维吉
* @date 2022/9/2 22:54
* @description 通用工具类
*/

vector<string> split(const string &str, const string &pattern);

template<typename ... Args>
String str_format(String format, Args ... args);

int index_of(std::string &text, std::string &pattern);

#endif

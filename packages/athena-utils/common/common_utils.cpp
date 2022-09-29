#include "common_utils.h"
#include <Arduino.h>
#include <vector>
#include <locale>
#include <iostream>

using namespace std;

/**
* @author 潘维吉
* @date 2022/9/2 22:54
* @description 通用工具类
*/

/**
 * 字符串分割转数组
 */
vector <string> split(const string &str, const string &pattern) {
    vector <string> res;
    if (str == "")
        return res;
    //在字符串末尾也加入分隔符，方便截取最后一段
    string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != strs.npos) {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        // 去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}

int indexOf(std::string &text, std::string &pattern) {
    // where appears the pattern in the text?
    std::string::size_type loc = text.find(pattern, 0);
    if (loc != std::string::npos) {
        return loc;
    } else {
        return -1;
    }
}
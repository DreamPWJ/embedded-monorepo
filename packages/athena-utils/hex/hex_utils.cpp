#include "hex_utils.h"

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

/**
* @author 潘维吉
* @date 2022/9/13 9:17
* @description HEX文件类型工具类
*/

/**
 * Hex转String类型
 */
std::string hex_to_string(const std::string& str)
{
    std::string result;
    for (size_t i = 0; i < str.length(); i += 2)
    {
        std::string byte = str.substr(i, 2);
        char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
        result.push_back(chr);
    }
    return result;
}

/**
 * String转Hex类型
 */
std::string string_to_hex(const std::string& data)
{
    const std::string hex = "0123456789ABCDEF";
    std::stringstream ss;

    for (std::string::size_type i = 0; i < data.size(); ++i)
        ss << hex[(unsigned char)data[i] >> 4] << hex[(unsigned char)data[i] & 0xf];
    std::cout << ss.str() << std::endl;
    return ss.str();
}
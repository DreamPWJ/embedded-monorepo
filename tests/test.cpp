#include "stdio.h"
#include <ctime>
#include <iostream>
#include <sstream>

using namespace std;

void test_json_file() {
    // 参考文章： https://github.com/nlohmann/json

}

// CLion运行单个c和c++文件(.c.cpp) 主要用于验证和测试功能:  https://www.cnblogs.com/wozen/p/15376304.html
int main(void) {
    //设置程序为中文编码
    setlocale(LC_ALL, "zh-CN");
    system("chcp 65001");

    printf("单文件运行测试 \n");

    test_json_file();

}



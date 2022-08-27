// CLion运行单个c和c++文件(.c.cpp):  https://www.cnblogs.com/wozen/p/15376304.html

#include "stdio.h"
#include <ctime>
#include <iostream>

using namespace std;

void test_json_file() {
  // 参考文章： https://stackoverflow.com/questions/32205981/reading-json-files-in-c

}

int main(void) {
    //设置程序为中文编码
    setlocale(LC_ALL, "zh-CN");
    system("chcp 65001");

    printf("单文件运行测试 \n");

    test_json_file();

/*    int overtime = 3;// 超时时间 秒
    time_t startA = 0, endA = 0;
    double costA; // 时间差 秒

    time(&startA);

    _sleep(5 * 1000); //延时5秒
    time(&endA);
    costA = difftime(endA, startA);
    printf("电机正向执行耗时：%f \n", costA);
    if (costA >= overtime) {
        printf("超时了 \n");
    }
    */
}



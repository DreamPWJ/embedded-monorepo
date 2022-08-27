// CLion运行单个c和c++文件(.c.cpp):  https://www.cnblogs.com/wozen/p/15376304.html

#include "stdio.h"
#include <ctime>
#include <iostream>

using   namespace   std;

int main(void ) {
    setlocale(LC_ALL,"zh-CN"); //设置程序为中文编码
    printf("666 \n");
    time_t startA = 0, endA = 0;
    double costA; // 时间差 秒

    time(&startA);

    _sleep(3*1000); //延时5秒
    time(&endA);
    costA = difftime(endA, startA);
    printf("电机正向执行耗时：%f \n", costA);
}
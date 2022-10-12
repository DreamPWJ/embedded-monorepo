#include <string>
#include <iostream>

using namespace std;

/**
* @author 潘维吉
* @date 2022/9/20 9:11
* @description 业务常量类
*/

class BizConstants {

public:
    static const int AGE = 18;  // c++只允许像int类型的静态变量直接在头文件里面初始化
    static const string NAME; // cpp文件内实现初始化

};


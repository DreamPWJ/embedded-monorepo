## 车位锁控制器嵌入式项目

### PlatformIO新一代嵌入式平台开发esp32单片机和Arduino框架

#### 参考文档

https://zhuanlan.zhihu.com/p/138214988
https://docs.platformio.org/en/latest/integration/ide/clion.html

#### PlatformIO安装

curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o get-platformio.py
python3 get-platformio.py
Windows用户, 把C:\Users\用户名\.platformio\penv\Scripts; 加到Path环境变量里才能生效 注意用户名不能有特殊符号
初始化项目命令 pio -c clion init --ide clion 并且自动生成CMakeListsPrivate.txt文件和下载项目依赖
在CLion中MinGW 和 CMake中配置编译与构建相关信息 指定C和C++编译器的exe文件

#### PlatformIO丰富的第三方库

https://registry.platformio.org/search?t=library

#### 打开Terminal,输入platformio device monitor,就可以打开串口监视器,查看串口输出.

#### 打开Terminal,输入pio -c clion home,就可以打开PlatformIO Home.

#### Espressif 32: development platform for PlatformIO 代码示例

https://github.com/platformio/platform-espressif32

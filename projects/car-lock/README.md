## 车位锁控制器嵌入式项目

### PlatformIO新一代嵌入式平台开发esp32单片机和Arduino框架

#### 参考文档

https://zhuanlan.zhihu.com/p/138214988
https://docs.platformio.org/en/latest/integration/ide/clion.html

#### MinGW 是最小的C、C++编译环境 gcc、g++。 验证命令 gcc -v

https://www.mingw-w64.org/ 与 https://sourceforge.net/projects/mingw-w64/files/  下载x86_64-posix-seh
并bin所在目录添加环境变量到Path中

#### PlatformIO安装

- curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o
- get-platformio.py
- python3 get-platformio.py
- Windows用户, 把C:\Users\用户名\.platformio\penv\Scripts; 加到Path环境变量里才能生效 注意用户名不能有特殊符号
- 初始化项目命令 pio -c clion init --ide clion 并且自动生成CMakeListsPrivate.txt文件和下载项目依赖
- 在CLion中MinGW 和 CMake中配置编译与构建相关信息 指定C和C++编译器的exe文件
- 升级版本： platformio upgrade

#### PlatformIO丰富的第三方库

https://registry.platformio.org/search?t=library

#### 打开Terminal,输入platformio device monitor,就可以打开串口监视器,查看串口输出.

#### 打开Terminal,输入pio -c clion home,就可以打开PlatformIO Home.

#### Espressif 32: development platform for PlatformIO 代码示例

https://github.com/platformio/platform-espressif32

#### .env文件使用方式示例

获取自定义多环境变量宏定义
define XSTR(x) #x
define STR(x) XSTR(x)
const char *env_app_version = STR(ENV_APP_VERSION);
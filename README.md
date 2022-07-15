## 嵌入式单片机程序monorepo单体式仓库 Embedded Development

### 项目代号: athena(雅典娜 智慧女神) 愿景: 使项目更易于复用迭代维护扩展、分离关注点并避免代码重复

### monorepo 最主要的好处是统一的工作流和共享代码, 兼顾通用性和独立性之间的最佳平衡点, 统一最佳实战只需搭建一套脚手架, 统一管理(规范、配置、开发、联调、构建、测试、发布等)多个包

### 目录结构

- packages: 可复用的基础通用包
- projects-*: 项目业务包
- templates: 自定义灵活高效的代码生成器
- scripts: 自定义脚本 管理复杂多项目
- docs: 项目文档
- examples: 示例代码 常用代码模板和代码块提炼
- tests: 测试模块

### 开发工具: CLion

### 开发语言: C 、C++

### MCU单片机类型: STM32(内核ARM)或ESP32(内核RISC-V)

### 嵌入式硬件平台: Arduino

### 云平台: 阿里云IoT物联网平台

### MQTT服务器: EMQX

#### 用于嵌入式 C/C++ 开发的新一代工具集PlatformIO 是新一代嵌入式开发生态系统  安装执行 python get-platformio.py

#### 搭建参考文章

https://blog.jetbrains.com/clion/2019/02/clion-2019-1-eap-clion-for-embedded-development-part-iii/
https://zhuanlan.zhihu.com/p/63672432
https://zhuanlan.zhihu.com/p/145801160

#### STM32CubeMX，用于配置 STM32 微控制器的图形工具

https://www.st.com/en/development-tools/stm32cubemx.html#getsoftware-scroll

#### ARM 工具链arm-none-eabi-gcc  用来编译ARM程序的交叉编译工具链

#### 注意Windows上安装完成选择添加到环境变量中, 否则无法通过CMake编译, 找不到C编译器！！！

https://developer.arm.com/downloads/-/gnu-rm

#### OpenOCD芯片调试器 将代码写入MCU中调试刷机

https://gnutoolchains.com/arm-eabi/openocd/

#### MinGW是最小的C、C++编译环境 gcc、g++。 验证命令 gcc -v

https://www.mingw-w64.org/

### 基于阿里云IoT物联网平台

https://help.aliyun.com/document_detail/30522.html
https://help.aliyun.com/document_detail/146228.html
https://help.aliyun.com/document_detail/254820.html
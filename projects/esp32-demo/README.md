## PlatformIO新一代嵌入式平台开发esp32单片机和Arduino框架示例

#### 参考文档

https://zhuanlan.zhihu.com/p/138214988
https://docs.platformio.org/en/latest/integration/ide/clion.html

#### PlatformIO安装

curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o get-platformio.py
python3 get-platformio.py
Windows用户,把C:\Users\用户名\.platformio\penv\Scripts; 加到Path环境变量里才能生效
初始化项目命令 pio -c clion init --ide clion

#### PlatformIO丰富的第三方库

https://registry.platformio.org/search?t=library

#### 打开Terminal,输入platformio device monitor,就可以打开串口监视器,查看串口输出.

#### 打开Terminal,输入pio -c clion home,就可以打开PlatformIO Home.

#### Espressif 32: development platform for PlatformIO 代码示例

https://github.com/platformio/platform-espressif32

#### ESP单片机SoftAP 配网和 Bluetooth Low Energy 配网

https://docs.espressif.com/projects/espressif-esp-moonlight/zh_CN/latest/networkconfig.html

#### HARDWARE: ESP32C3 160MHz, 320KB RAM, 4MB Flash

#### ESP乐鑫工具esptool是ROM Bootloader一级bootloader通讯，从而实现的 固件烧录，flash 擦除，flash 读取，读 MAC 地址，读 flash id ，elf 文件转 bin 等常用功能

python -m pip install esptool

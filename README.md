/*****************************************************************************/

为了帮助用户“简单快速”开发 LoRaWAN 设备（如：水表，燃气表，GPS 定位器，温湿度，阀控，地磁，烟感。。。。）
锐米公司(www.rimelink.com)开源了 ArduinoLoRaLibray 它基于 Arduino 平台（包括 UNO 和 Pro Mini），驱动 LoRaWAN 模组。

ArduinoLoRaLibrary.ino 是测试代码示例，它包含 TestClassA 和 TestClassC，分别测试 LoRaWAN 模组在 Class A 和 Class C
这 2 种模式下：发送数据，接收数据，打印信号质量（RSSI 和 SNR）和 ACK 等。

在使用本库之前，请确保您手上有完整的 LoRaWAN 系统，这通常包括 LoRaWAN 基站和 loraserver 以及 lora app
请参考《LoRa Server 配置与管理》  https://blog.csdn.net/jiangjunjie_2005/article/details/96169551

本代码基于 LoRaWAN 开发板测试通过，开发板本身也是开源设计，请链接：http://www.rimelink.com/pd.jsp?id=17#_pp=2_306


/*****************************************************************************/

将 lora.cpp 和 lora.h 替换下面路径的“旧”文件
C:\Users\Administrator\Documents\Arduino\libraries\ArduinoLoRaLibrary-1.1


/*****************************************************************************/

2021-07-06
1）节省了 RAM 空间 50% 以上；
2）添加了 TestClassA 和 TestClassC；
3）扩展了 LoRa.write() 允许发送 Unconfirmed / Confirmed；
4）扩展了 LoRa.read() 能接收 RSSI 和 SNR 信号质量，接收 ACK
5）改进了匹配 LoRa 帧算法效率，修改了一些已知的错误

2019-10-27
初次发布，能正确对接 LoRa 模组。

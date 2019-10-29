# NUCLEO_2.4inchTFT_uGUI
一个运行在nucleo_f103rb上的使用2.4英寸的arduinoTFT彩屏（使用ili9341驱动），结合uGUI的演示项目，目前触摸功能还不能使用
本程序基于hal开发，请用cubeMx先重新生成项目，再编译。
2.4英寸TFT屏资料：
http://www.lcdwiki.com/zh/2.4inch_Arduino_Display#.E4.BA.A7.E5.93.81.E5.9B.BE.E7.89.87
该屏使用8080的8位总线模式。

屏驱动来自于库：
https://github.com/dizcza/LCD_ILI9341_TouchScreen

原项目是基于NUCLEO-F446板，移植到F103上后触摸部分不能工作，该问题尚未解决。

uGui来自于库：
https://github.com/achimdoebler/UGUI


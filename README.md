# ESP32 Composite Video Demo

This program is an example how to use [ESP32 Composite Video library](https://github.com/aquaticus/esp32_composite_video_lib).

![Demo Slides](https://github.com/aquaticus/esp32_composite_video_lib/raw/main/doc/esp32_composite_video_demo.gif)

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

## Requirements

It should work with any EPS32 board with Tensilica core. The test environment was as follows:

| Name | Value  |
|------|--------|
|Chip  |ESP32   |
|Cores |2       |
|RAM   |520 KiB |
|FLASH |4 MiB   |
|Clock |240 MHz |

The project was compiled using [ESP32-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#introduction) v4.4.

## Connection

Connect **GPIO25** to composite video input and board **GND** to input GND.

If you use a CINCH (RCA) cable, the video signal goes to the pin in the middle, and GND to the outside metal part.

![ESP32 Board](https://github.com/aquaticus/esp32_composite_video_lib/raw/main/doc/esp32_board_cinch.png)

# Build

Make sure you have ESP32-IDF environment installed.

First clone the repository with 2 submodules: LVGL and Composite Video Library.

```bash
git clone --recurse-submodules https://github.com/aquaticus/esp32_composite_video_demo
```
Build

```bash
cd esp32_composite_video_demo
idf.py build
```

and upload to the board

```bash
idf.py flash
```

# Notes

* By default, the program generates PAL signal.
* If you disable LVGL support, Philips PM5544 test image is displayed.
* Using 16 bit LVGL color depth disables auxiliary buffer (to save memory); this makes tearing effect visible.
* Compilation optimization level must be set to *performance* (`-O2`). This is the default setting for demo project.

## How to create LVGL font
```bash
npm install -g lv_font_conv
lv_font_conv --font MicrosoftYaHei-01.ttf --size 20 --bpp 4 --range 0x20-0x7F,0x4E00-0x5FFF --no-compress -o lv_font_msyh_20.c --format lvgl
```
--range：指定字符集范围  
- 0x20-0x7F：ASCII字符集  
- 0x4E00-0x5FFF：中文字符集  

 --bpp 4：4位颜色深度，数值越大字体越平滑，但是生成的C代码体积也会暴涨  
 --no-compress：不压缩生成的C代码  
 --format lvgl：生成的C代码格式为LVGL  
 
 若使用ttc字体，可能会遇到错误：  
```bash
Cannot load font "simsun.ttc": Unsupported OpenType signature ttcf
```
报错含义：
lv_font_conv 依赖的 opentype.js 只能解析 单个 OpenType 字体，而 simsun.ttc / msyh.ttc 是 TrueType Collection（ttcf 签名），一个文件里打包了多套字重/字形，所以直接扔进去会提示 Unsupported OpenType signature ttcf。
解决方案：使用在线字体快拆工具 https://transfonter.org/ttc-unpack  

## FlintESPJVM
This project implements [FlintJVM](https://github.com/FlintVN/FlintJVM) and is based on the ESP-IDF framework, It supports running and debugging java code on ESP32.

![demo](images/demo1.avif)
## Features
- Supports most Java bytecode instructions.
- Supports debugging with [FlintJVM Debug](https://marketplace.visualstudio.com/items?itemName=ElectricThanhTung.flintjvm-debugger) VS Code extension via serial port.
## How to use
### 1. Boards are supported
- Generic ESP32 Boards.
- ESP32-C3FH4.
- ESP32-C6FH4.
- ESP32-C6FH8.
- ESP32-S2FN4R2.
- ESP32-S3FH4R2.
- ESP32-S3N4RX.
- ESP32-S3N8RX.
- ESP32-S3N16RX.
### 2. Flash
- Visit [https://esp.flint.vn](https://esp.flint.vn) (ESP web tool) to quickly flash the FlintESPJVM firmware onto your board and try it out.
- You can also use ESP-IDF to build this project yourself and flash it to your board.
### 3. Build java project to run on FlintESPJVM
- To build a java project for FlintESPJVM you need to build your project with the FlintJDK module to be able to use the base java libraries compatible with FlintJVM and FlintESPJDK module if you want to access the peripherals/features of ESP32.
- To be able to run debugging for your java project. You need to install [FlintJVM Debug](https://marketplace.visualstudio.com/items?itemName=ElectricThanhTung.flintjvm-debugger) extension on VS Code and then use the [FlintUartServer](https://github.com/FlintVN/FlintUARTServer) tool as a converter between serial port and TCP/IP.
- Refer to the [FlintExample](https://github.com/FlintVN/FlintExample) for a FlintJVM java project basic example.
### 4. Clone repository
- Run the following command to clone this repository and include all dependent submodules:
```sh
> git clone --recurse-submodules https://github.com/FlintVN/FlintESPJVM.git
```
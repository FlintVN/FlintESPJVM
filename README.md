## FlintESPJVM
This project implement [FlintJVM](https://github.com/FlintVN/FlintJVM) and is based on ESP-IDF framework, It support running and debugging java code on ESP32.
## Features
- Supports most Java bytecode instructions.
- Supports debugging with [FlintJVM Debug](https://marketplace.visualstudio.com/items?itemName=ElectricThanhTung.flintjvm-debugger) VS Code extension via serial port.
## How to use
### 1. Hardware required
- This project requires an ESP32 chip that supports USB protocol such as ESP32-S2 or ESP32-S3... and has integrated PSRAM.
### 2. Clone repository
- Run the following command to clone this repository and include all dependent submodules:
```sh
> git clone --recurse-submodules https://github.com/FlintVN/FlintESPJVM.git
```
### 3. Build and flash
- Run `idf.py -p PORT flash` to build and flash the project. Or you can build this project using the [ESP-IDF](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) extension on VS Code.
### 3. Build java project to run on FlintESPJVM
- To build a java project for FlintESPJVM you need to build your project with the FlintJDK module to be able to use the base java libraries compatible with FlintJVM and FlintESPJDK module if you want to access the peripherals/features of ESP32.
- To be able to run debugging your java project. You need to install [FlintJVM Debug](https://marketplace.visualstudio.com/items?itemName=ElectricThanhTung.flintjvm-debugger) extension on VS Code and the [FlintUartServer](https://github.com/FlintVN/FlintUARTServer) tool as a converter between serial port and TCP/IP.
- Refer to [FlintExample](https://github.com/FlintVN/FlintExample) for a FlintJVM java project basic example.
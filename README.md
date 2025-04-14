## FlintESPJVM
FlintESPJVM brings the power of Java to ESP32 devices by implementing [FlintJVM](https://github.com/FlintVN/FlintJVM) on the ESP-IDF framework. This allows you to run and debug Java programs directly on the ESP32 hardware.

![demo](images/demo1.avif)
## Key Features
- Supports most Java bytecode instructions.
- Supports debugging with [FlintJVM Debug](https://marketplace.visualstudio.com/items?itemName=ElectricThanhTung.flintjvm-debugger) VS Code extension via serial port.
## How to use
### 1. Supported Boards
FlintESPJVM is compatible with a variety of ESP32 boards, including:
- Generic ESP32 Boards.
- ESP32-C3FH4.
- ESP32-C6FH4.
- ESP32-C6FH8.
- ESP32-S2FN4R2.
- ESP32-S3FH4R2.
- ESP32-S3N4RX.
- ESP32-S3N8RX.
- ESP32-S3N16RX.
### 2. Flashing the Firmware
To get FlintESPJVM up and running on your board:
- Quick Flash: Use the [ESP Web Tool](https://esp.flint.vn) for an easy, browser-based flashing experience.
- Manual Build: Prefer building it yourself? Utilize the ESP-IDF framework to compile and flash the project manually.
### 3. Building Java Projects for FlintESPJVM
To develop Java applications for FlintESPJVM:
- Project Setup: Use the [FlintJDK](https://github.com/FlintVN/FlintJDK) module to access core Java libraries compatible with FlintJVM. For ESP32-specific peripherals and features, add the [FlintESPJDK](https://github.com/FlintVN/FlintESPJDK) module.
- Debugging: Install the FlintJVM Debug extension in VS Code.

Refer to the [FlintExample](https://github.com/FlintVN/FlintExample) project, which provides a simple and clear template to help you get started with building Java apps for FlintJVM.
### 4. Cloning the Repository
- Run the following command to clone this repository and include all dependent submodules:
```sh
> git clone --recurse-submodules https://github.com/FlintVN/FlintESPJVM.git
```
---
*Elevate your ESP32 projects by harnessing the robustness of Java with FlintESPJVM.*
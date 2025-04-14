# Change Log
## V0.0.8
- Update FlintJVM to [V1.1.4](https://github.com/FlintVN/FlintJVM/releases/tag/V1.1.4) (Fix bug when handling finally block in exception).
- Update FlintJDK to [V0.0.6](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.6).
  - Implement the following functions for java.util.Arrays: sort, equals, deepEquals, compare, mismatch.
  - Support java.util.ArrayList, java.util.Vector and java.util.LinkedList.
- Update FlintESPJDK to [V0.0.4](https://github.com/FlintVN/FlintESPJDK/releases/tag/V0.0.4) (Reorganize packages (machine.* -> esp.machine and network -> esp.network)).
- Update native method to be compatibility with FlintESPJDK V0.0.4.
## V0.0.7
- Update FlintJVM to [V1.1.3](https://github.com/FlintVN/FlintJVM/releases/tag/V1.1.3).
- Update FlintJDK to [V0.0.5](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.5).
- Increase default stack size to 4KB.
- Update memory partitions (increased from 1MB to 1.5MB for FlintESPJVM).
## V0.0.6
- Update FlintJVM to [V1.1.2](https://github.com/FlintVN/FlintJVM/releases/tag/V1.1.2).
  - Fix bugs and implement more native methods for the java base library.
- Update FlintJDK to [V0.0.4](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.4).
  - Add some necessary classes and methods to the java base library.
## V0.0.5
- Update FlintJVM to V1.1.1.
- Support machine.spi.SPIMaster.
- Implement FlintAPI::System::reset mehtod to support peripheral reset.
## V0.0.4
- Support for boards:
  - ESP32-S3N4RX
  - ESP32-C3FH4
  - ESP32-C6FH4
  - ESP32-C6FH8
## V0.0.3
- Support for S3N16RX and S3N8RX boards.
## V0.0.2
- Stable communication between Flint Client and Flint Server.
- Change default UART baud rate from 2000000 to 921600.
- Support and add native methods for machine.gpio.Port class.
- Fix infinite loop bug in FlintAPI::System::print.
- Support and add native method for network.WiFi class.
- Runs on [FlintJVM V1.1.0](https://github.com/FlintVN/FlintJVM/releases/tag/V1.1.0), [FlintJDK V0.0.2](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.2) and [FlintESPJDK V0.0.2](https://github.com/FlintVN/FlintESPJDK/releases/tag/V0.0.2).
- Compatible with [FlintJVMDebug V1.1.0](https://github.com/FlintVN/FlintJVMDebug/releases/tag/V1.1.0) or higher.
## V0.0.1
- The first version is supported on ESP32, ESP32-S2 and ESP32-S3.
- Runs on [FlintJVM V1.0.0](https://github.com/FlintVN/FlintJVM/releases/tag/V1.0.0), [FlintJDK V0.0.1](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.1) and [FlintESPJDK V0.0.1](https://github.com/FlintVN/FlintESPJDK/releases/tag/V0.0.1).
- Compatible with [FlintJVMDebug V1.0.0](https://github.com/FlintVN/FlintJVMDebug/releases/tag/V1.0.0).
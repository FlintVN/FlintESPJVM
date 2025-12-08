# Change Log
## V0.3.2
- Update to [FlintJVM V2.3.3](https://github.com/FlintVN/FlintJVM/releases/tag/V2.3.3):
  - Fixed critical bug, stackPopDouble corrupted java stack frame.
  - Fixed bug in java.io.FileOutputStream.write method with FileDescriptor.out (Big impact on System.out.print).
  - Support FileDescriptor.sync native method.
- Update to compatible with [FlintJDK V1.2.0](https://github.com/FlintVN/FlintJDK/releases/tag/V1.2.0).
  - Add support for flint.machine.I2sMaster.
  - Add support for flint.machine.Adc.
  - Add support for flint.machine.Dac.
## V0.3.1
- Update to [FlintJVM V2.3.2](https://github.com/FlintVN/FlintJVM/releases/tag/V2.3.2):
  - Terminate all when any thread encounters an unhandled exception.
  - Fixed critical bug related to thread synchronization, monitorenter instruction corrupts the stack frame in case the lock object fails.
## V0.3.0
- Compatible with [FlintJDK V1.1.0](https://github.com/FlintVN/FlintJDK/releases/tag/V1.1.0).
- Update to [FlintJVM V2.3.1](https://github.com/FlintVN/FlintJVM/releases/tag/V2.3.1):
  - Support for specifying class path.
  - Fixed a critical bug related to Garbage Collection, objects that are still in use are deleted and objects that are no longer in use are not deleted.
  - Fix memory leak in ClassLoader class.
  - Fix the READ_EXCP_INFO command in the debugger, which returns an incorrect format when the exception message is null.
- Improved performance for SpiMaster.write and SpiMaster.readWrite, significantly reducing overhead time.
- Support flint.machine.SerialPort.
- Support flint.machine.I2cMaster.
- Support flint.machine.BitStream.
- Support flint.machine.OneWire.
## V0.2.0
- Update to be compatible with [FlintJDK V1.0.0](https://github.com/FlintVN/FlintJDK/releases/tag/V1.0.0) and [FlintESPJDK V0.0.9](https://github.com/FlintVN/FlintESPJDK/releases/tag/V0.0.9).
- Base on [FlintJVM V2.3.0](https://github.com/FlintVN/FlintJVM/releases/tag/V2.3.0).
## V0.1.1
- Change default UART baud rate from 921600 to 460800 to increase compatibility.
## V0.1.0
- Update FlintJVM to [V2.2.0](https://github.com/FlintVN/FlintJVM/releases/tag/V2.1.1) and FlintJDK to [V0.1.0](https://github.com/FlintVN/FlintJDK/releases/tag/V0.1.0):
  - Rename Character.toLower to Character.toLowerCase and Character.toUpper to Charactor.toUpperCase (both FlintJDK and FlintJVM).
  - Fix issue for DBG_CMD_READ_FIELD in debugger when field is array (FlintJVM).
  - Support for java.io.File, java.io.FileOutputStream and java.io.FileImputStream classes (both FlintJDK and FlintJVM).
  - Add missing methods to java.lang.Math class (FlintJDK).
  - Add support for Map.of, Map.ofEntries and Map.copyOf methods (FlintJDK).
  - Add support for String.compareToIgnoreCase method (FlintJDK).
## V0.0.15
- Update FlintJVM to [V2.1.1](https://github.com/FlintVN/FlintJVM/releases/tag/V2.1.1):
  - Fix stuck in FMutex::lock with 1 core chips.
## V0.0.14
- Update FlintJVM to [V2.1.0](https://github.com/FlintVN/FlintJVM/releases/tag/V2.1.0):
  - Support call to java method in native interface.
  - Fix bug where existing values ​​could not be found in FDict.
  - Fix memory leak.
  - Class.isHidden return false instead of throwing exception.
- Update FlintJDK to [V0.0.11](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.11):
  - Fix bug for Integer.toString() and Long.toString() with MIN_VALUE.
  - Add support Integer.getChars, Long.getChars and StringUTF16.getChars methods.
  - Add java.lang.IllegalMonitorStateException and java.lang.StackOverflowError.
  - Fix bug in Class.descriptorString method with void type.
## V0.0.13
- Update FlintJVM to V2.0.1 to fix the following bugs:
  - Fix bug in FNIEnv::newObjectArray.
  - Fix Class.initClassName native method do not assign a value to Class.name.
  - Minor performance optimizations for invoke instructions.
  - Check static initialization before executing invokevirtual and invokeinterface instructions.
## V0.0.12
- Update to compatible with [FlintJVM V2.0.0](https://github.com/FlintVN/FlintJVM/releases/tag/V2.0.0).
- Update FlintJDK to [V0.0.10](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.10).
- Update FlintESPJDK to [V0.0.8](https://github.com/FlintVN/FlintESPJDK/releases/tag/V0.0.8).
- Compatible with [FlintJVMDebug V2.0.0](https://github.com/FlintVN/FlintJVMDebug/releases/tag/V2.0.0).
## V0.0.11
- Fix bug not working on ESP32-C3.
- Support esp.machine.Pin.toggle().
- Update to [FlintJVM V1.1.7](https://github.com/FlintVN/FlintJVM/releases/tag/V1.1.7).
- Compatible with [FlintJDK V0.0.9](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.9).
## V0.0.10
- Update to [FlintJVM V1.1.6](https://github.com/FlintVN/FlintJVM/releases/tag/V1.1.6)
  - Fix VM crash when native method not found.
  - Fix stack trace with wrong PC address when debugging.
  - Fix stuck at Flint::terminate for single core chips.
  - Fixed VM throwing wrong type for some exceptions or not displaying messages.
- Update to [FlintJDK V0.0.8](https://github.com/FlintVN/FlintJDK/releases/tag/V0.0.8).
- Update to [FlintESPJDK V0.0.6](https://github.com/FlintVN/FlintESPJDK/releases/tag/V0.0.6).
- Fix WiFi.getAPinfo native method returning wrong value.
- Update native method for esp.machine.SpiMaster to match with new FlintESPJDK and check validity for SPI Pin.
- Implement esp.machine.Pin validity check when Pin.setMode is called.
- Fix bug not working on ESP32-C6.
## V0.0.9
- Update FlintJVM to [V1.1.5](https://github.com/FlintVN/FlintJVM/releases/tag/V1.1.5)
  - Fix method not found in invokeInterface (bridge method).
  - Support lazy loading for method code (Reduce RAM consumption).
  - Fix java.lang.reflect.Array.get returning short instead of character with character input array.
  - Fix crash when class file contains long ConstUtf8 string.
  - Eliminate the impact of breakpoints on performance when debugging.
  - Temporarily dropping support for java.math.BigInteger for future changes and improvements.
  - Reduce RAM consumption.
- Reduce DEFAULT_STACK_SIZE from 10KB to 5KB.
- Update esp.machine.Pin and esp.machine.Port to increase performance.
- Update memory partitions (reduce from 1.5MB to 1MB for FlintESPJVM).
- Check the reset cause to decide whether to run into the main java code or not.
- Update FlintESPJDK to [V0.0.5](https://github.com/FlintVN/FlintESPJDK/releases/tag/V0.0.5).
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
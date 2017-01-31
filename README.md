# I2C Demo #
## Overview ##
This code demonstrates setting an Arduino Uno up as an I2C slave and reading a JSON string from a Raspberry Pi, as the I2C master, using Windows 10 IoT Core and c#.

## Wiring ##

Connect SCL from the Raspberry Pi to A5 on the Arduino Uno
Connect SDA from the Raspberry Pi to A4 on the Arduino Uno
Connect Ground from the Raspberry Pi to Ground on the Arduino Uno

Optional (highly recommended):
Since the Arduino Uno is a 5V micro-controller and the raspberry Pi is 3V3, it is important to keep the I2C pins at 3V3.

Connect a 4.7K Ω resistor between SCL and 3.3 V on the Raspberry Pi.
Connect a 4.7K Ω resistor between SDA and 3.3 V on the Raspberry Pi.

## Setting Up ##
### Uno ###
Load he **i2c_slave.ino** sketch to the Uno. Open the Serial Port Monitor to watch the serial events.

### Raspberry Pi ###
Load the solution into Visual Studio 2015, and set the configuration to ARM, set the target to Remote Machine and specify your Raspberry Pi. At this point you can either start the solution under the debugger (F5) or deploy the application and start it remotely from the portal page on your Raspberry Pi.

## Output ##
The display on the Raspberry Pi will output the JSON string every 5 seconds. The Arduino will output messages on the serial port as it reds the imaginary sensor and as it sends and receives data over the I2C bus.

## Notes ##
- The Wire library on the Arduino platform limits data between devices to 32 bytes. This is the default size which can be increased to 64 bytes, which is the maximum for the Atmel 328. Different platforms may have different defaults and maximum values. Read the documentation for the specific platform. This library assumes the data being transferred back and forth will exceed 32 bytes and therefore transfers the data in blocks.
- In this example, the Uno assumes that the master will send 2 bytes in each request. The first byte is the command and the second byte is the command parameter. In some cases, the command parameter is ignored.
- Each response to a command will return a one byte result except for the get block command which will return the number of bytes equal to BLOCK_SIZE (default is 32).

## Commands ##
- 1 => Lock/unlock the data. The command parameter is either 0 to unlock or 1 to lock.
- 2 => Returns the size of a block. No command parameter (set parameter = 0).
- 3 => Returns the number of blocks in the buffer. No command parameter (set parameter = 0).
- 4 => Returns the size in bytes of the JSON string. No command parameter (set parameter = 0).
- 5 => Read the nth block from the buffer where the command parameter is n and n starts at 1.
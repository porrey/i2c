# i2c
Demonstrates setting an Arduino Uno up as an I2C slave and reading a JSON string from RPi3, as the I2C master, using Windows 10 IoT Core and c#.

# Wiring

Connect SCL from the Raspberry Pi to A5 on the Arduino Uno
Connect SDA from the Raspberry Pi to A4 on the Arduino Uno
Connect Ground from the Raspberry Pi to Ground on the Arduino Uno

Optional:
Connect a 4.7K resistor between SCL and 3.3 V on the Raspberry Pi.
Connect a 4.7K resistor between SDA and 3.3 V on the Raspberry Pi.
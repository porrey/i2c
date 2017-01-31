#include <Wire.h>
#include "ArduinoJson.h"
#include <math.h>

// ***
// *** Set the block size to match the
// *** default i2c buffer size of 32 (see
// *** Wire.h).
// ***
#define BLOCK_SIZE 32

// ***
// *** Default slave address. Change this to
// *** any address that does not conflict
// *** with other i2c devices on the same bus.
// ***
#define I2C_SLAVE_ADDRESS 0x26

// ***
// *** Define a buffer for received data. Need volatile because
// *** we are updating this buffer outside the normal loop.
// ***
const uint8_t OUT_BUFFER_SIZE = 128;
volatile byte _outboundBuffer[OUT_BUFFER_SIZE];
uint8_t _stringLength = 0;
uint8_t _totalBlocks = 0;

// ***
// *** Holds the last command and parameter sent by the master.
uint8_t _lastCommand = 0;
uint8_t _lastParameter = 0;

// ***
// *** Used to lock the data while reading.
// ***
volatile bool _readLock = false;

void setup()
{
  // ***
  // *** Initialize the USB Serial port
  // ***
  Serial.begin(115200);
  Serial.println("Initializing...");

  Serial.print("i2c buffer length is "); Serial.println(BUFFER_LENGTH);

  // ***
  // *** Initialize i2c in slave mode.
  // ***
  Wire.begin(I2C_SLAVE_ADDRESS);

  // ***
  // *** Attach the receive and request events.
  // ***
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.println("Initialization complete.");
}

void loop()
{
  if (!_readLock)
  {
    Serial.println("Reading sensors...");

    // ***
    // *** Clear the output buffer so there are no extra
    // *** characters at the end of a JSON string.
    // ***
    clearOutputBuffer();

    // ***
    // *** Read the (imaginary) sensor(s).
    // ***
    StaticJsonBuffer<200> jsonBuffer;

    // ***
    // *** Create a JSON object with varying data (using
    // *** random).
    // ***
    JsonObject& rootObject = jsonBuffer.createObject();
    JsonObject& sensorObject = rootObject.createNestedObject("sensors");
    JsonObject& envObject = sensorObject.createNestedObject("environment");
    envObject["temperature"] = random(100, 10000) / 100.0;
    envObject["humidity"] = random(100, 10000) / 100.0;
    envObject["pressure"] = random(100, 10000) / 100.0;
    JsonObject& gpsObject = sensorObject.createNestedObject("gps");
    gpsObject["lat"] = random(100, 10000) / 100.0;
    gpsObject["long"] = random(100, 10000) / 100.0;

    // ***
    // *** Send to the Serial port.
    // ***
    Serial.print("json => "); rootObject.printTo(Serial); Serial.println();

    // ***
    // *** Write the JSON string to the output buffer. Note
    // *** there is no logic in this demo to prevent the buffer
    // *** from being cleared or updated while the i2c bus is
    // *** reading it.
    // ***
    rootObject.printTo(_outboundBuffer, OUT_BUFFER_SIZE);

    // ***
    // *** Show the actual length of the string. Useful for debugging if the string
    // *** is too large for the i2c buffers.
    // ***
    _stringLength = String((const char *)_outboundBuffer).length();
    Serial.print("Length of string is "); Serial.print(_stringLength); Serial.println(" byte(s).");

    _totalBlocks = (uint8_t)ceil((float)_stringLength / (float)BLOCK_SIZE);
    Serial.print("Total number of blocks is "); Serial.print(_totalBlocks); Serial.print(" (block size is "); Serial.print(BLOCK_SIZE); Serial.println(").");

    Serial.println();

    // ***
    // *** Delay 2 seconds.
    // ***
    delay(2000);
  }
  else
  {
    // ***
    // *** Cannot update the data while there is a read lock.
    // ***
    Serial.println("The data is locked and cannot be updated.");
    delay(500);
  }
}

void receiveEvent(uint8_t byteCount)
{
  Serial.print("receiveEvent() was called with "); Serial.print(byteCount); Serial.println(" byte(s).");

  // ***
  // *** We are expecting 2 bytes from the master. The first byte is the command.
  // *** the second byte is the command parameter.
  // ***
  // *** Commands:
  // ***
  // *** 1 => Lock/unlock the data. The command parameter is either 0 to unlock or 1 to lock.
  // *** 2 => Returns the size of a block. No command parameter (set parameter = 0).
  // *** 3 => Returns the number of blocks in the buffer. No command parameter (set parameter = 0).
  // *** 4 => Returns the size in bytes of the JSON string. No command parameter (set parameter = 0).
  // *** 5 => Read the nth block from the buffer where the command parameter is n and n starts at 1.
  // ***
  if (byteCount == 2)
  {
    // ***
    // *** Store the command and parameter.
    // ***
    _lastCommand = Wire.read();
    _lastParameter = Wire.read();
  }
  else
  {
    // ***
    // *** Clear the receive buffer.
    // ***
    while (Wire.available())
    {
      Wire.read();
    }

    // ***
    // *** Reset
    // ***
    _lastCommand = 0;
    _lastParameter = 0;
  }
}

void requestEvent()
{
  Serial.println("requestEvent() was called.");

  uint8_t startIndex = 0;

  switch (_lastCommand)
  {
    case 1:
      if (_lastParameter == 0)
      {
        Serial.println("Unlocking data.");
        _readLock = false;
        Wire.write(1);
      }
      else if (_lastParameter == 1)
      {
        Serial.println("Locking data.");
        _readLock = true;
        Wire.write(1);
      }
      else
      {
        Wire.write(0);
      }
      break;
    case 2:
      // ***
      // *** Sending 1 byte...
      // ***
      Wire.write(BLOCK_SIZE);
      break;
    case 3:
      // ***
      // *** Sending 1 byte...
      // ***
      Wire.write(_totalBlocks);
      break;
    case 4:
      // ***
      // *** Sending 1 byte...
      // ***
      Wire.write(_stringLength);
      break;
    case 5:
      // ***
      // *** Sending 32 bytes...
      // ***
      startIndex = (_lastParameter - 1) * 32;

      for (uint8_t i = startIndex; i < startIndex + 32; i++)
      {
        Wire.write(_outboundBuffer[i]);
      }
      break;
    case 6:
      // ***
      // *** Sending 1 byte...
      // ***
      Wire.write((byte)_readLock);
      break;
  }
}

void clearOutputBuffer()
{
  // ***
  // *** Set all the bytes to 0
  // ***
  for (int i = 0; i < OUT_BUFFER_SIZE; i++)
  {
    _outboundBuffer[i] = 0;
  }
}

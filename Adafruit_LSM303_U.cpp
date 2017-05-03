/***************************************************************************
  This is a library for the LSM303 Accelerometer and magnentometer/compass

  Designed specifically to work with the Adafruit LSM303DLHC Breakout

  These displays use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#ifdef __AVR_ATtiny85__
  #include "TinyWireM.h"
  #define Wire TinyWireM
#else
  #include <Wire.h>
#endif

#include <limits.h>

#include "Adafruit_LSM303_U.h"

/* enabling this #define will enable the debug print blocks
#define LSM303_DEBUG
*/

static float _lsm303Accel_MG_LSB     = 0.001F;   // 1, 2, 4 or 12 mg per lsb
static float _lsm303Mag_Gauss_LSB_XY = 1100.0F;  // Varies with gain
static float _lsm303Mag_Gauss_LSB_Z  = 980.0F;   // Varies with gain

/***************************************************************************
 ACCELEROMETER
 ***************************************************************************/
/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
void Adafruit_LSM303_Accel_Unified::write8(byte address, byte reg, byte value)
{
  Wire.beginTransmission(address);
  #if ARDUINO >= 100
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)value);
  #else
    Wire.send(reg);
    Wire.send(value);
  #endif
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
byte Adafruit_LSM303_Accel_Unified::read8(byte address, byte reg)
{
  byte value;

  Wire.beginTransmission(address);
  #if ARDUINO >= 100
    Wire.write((uint8_t)reg);
  #else
    Wire.send(reg);
  #endif
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  #if ARDUINO >= 100
    value = Wire.read();
  #else
    value = Wire.receive();
  #endif  
  Wire.endTransmission();

  return value;
}

/**************************************************************************/
/*!
    @brief  Reads the raw data from the sensor
*/
/**************************************************************************/
void Adafruit_LSM303_Accel_Unified::read()
{
  // Read the accelerometer
  Wire.beginTransmission((byte)LSM303_ADDRESS_ACCEL);
  #if ARDUINO >= 100
    Wire.write(LSM303_REGISTER_ACCEL_OUT_X_L_A | 0x80);
  #else
    Wire.send(LSM303_REGISTER_ACCEL_OUT_X_L_A | 0x80);
  #endif
  Wire.endTransmission();
  Wire.requestFrom((byte)LSM303_ADDRESS_ACCEL, (byte)6);

  // Wait around until enough data is available
  while (Wire.available() < 6);

  #if ARDUINO >= 100
    uint8_t xlo = Wire.read();
    uint8_t xhi = Wire.read();
    uint8_t ylo = Wire.read();
    uint8_t yhi = Wire.read();
    uint8_t zlo = Wire.read();
    uint8_t zhi = Wire.read();
  #else
    uint8_t xlo = Wire.receive();
    uint8_t xhi = Wire.receive();
    uint8_t ylo = Wire.receive();
    uint8_t yhi = Wire.receive();
    uint8_t zlo = Wire.receive();
    uint8_t zhi = Wire.receive();
  #endif    

  // Shift values to create properly formed integer (low byte first)
  _accelData.x = (int16_t)(xlo | (xhi << 8)) >> 4;
  _accelData.y = (int16_t)(ylo | (yhi << 8)) >> 4;
  _accelData.z = (int16_t)(zlo | (zhi << 8)) >> 4;
}

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/
 
/**************************************************************************/
/*!
    @brief  Instantiates a new Adafruit_LSM303 class
*/
/**************************************************************************/
Adafruit_LSM303_Accel_Unified::Adafruit_LSM303_Accel_Unified(int32_t sensorID) {
  _sensorID = sensorID;
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/
 
/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
bool Adafruit_LSM303_Accel_Unified::begin()
{
  // Enable I2C
  Wire.begin();

  // Enable the accelerometer (100Hz)
  write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, 0x57);
  
  // LSM303DLHC has no WHOAMI register so read CTRL_REG1_A back to check
  // if we are connected or not
  uint8_t reg1_a = read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A);
  if (reg1_a != 0x57)
  {
    return false;
  }  
  
  return true;
}

/**************************************************************************/
/*! 
    @brief  Gets the most recent sensor event
*/
/**************************************************************************/
bool Adafruit_LSM303_Accel_Unified::getEvent(sensors_event_t *event) {
  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));
  
  /* Read new data */
  read();

  event->version   = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type      = SENSOR_TYPE_ACCELEROMETER;
  event->timestamp = millis();
  event->acceleration.x = _accelData.x * _lsm303Accel_MG_LSB * SENSORS_GRAVITY_STANDARD;
  event->acceleration.y = _accelData.y * _lsm303Accel_MG_LSB * SENSORS_GRAVITY_STANDARD;
  event->acceleration.z = _accelData.z * _lsm303Accel_MG_LSB * SENSORS_GRAVITY_STANDARD;

  return true;
}

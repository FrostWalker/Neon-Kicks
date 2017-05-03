/**********************************************/
/*    VUW MDDN 351 PROJECT 1                  */
/*    AUTHOR: SAM PAPPAFLORATOS               */
/*    BOARD: ARDUINO GEMMA                    */
/*    PROGRAMMER: USBTinyISP                  */
/*    CPU: ATTiny85                           */
/*    DATE: 2015.08.20                        */
/**********************************************/

#include "Adafruit_Sensor.h" //Universal Sensor library
#include "Adafruit_LSM303_U.h" //Accelerometer + Magnometer library

#include "Adafruit_NeoPixel.h" //Neopixel library

//Initiate the averaging variables
const char numReadings = 11; //Smoothing value (Higher = smoother)
char readIndex = 0; //Array counter
short readings[3][numReadings], readingsTotal[3]; //Averaging Arrays

//Initiate the Neopixel variables
const char numPixels = 44; //Number of Neopixels
const char pin = 1; //Data pin number
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, pin, NEO_GRB + NEO_KHZ800); //The Neopixels

//Initiate the sensor variable
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321); //The Accelerometer

//Set up the sketch
void setup(void) {
  //Start the Accelerometer
  accel.begin();

  //Start the Neopixels
  strip.begin();
  strip.setBrightness(255);
  strip.show();

  //Create a zero-set array for averaging
  for (char i = 0; i < 3; i++) {
    memset(readings[i], 0, sizeof(readings[i]));
  }
}

void loop(void) {
  //Initiate the value array for XYZ
  short val[3];

  //Read sensor data
  sensors_event_t event;
  accel.getEvent(&event);

  //Set the base XYZ values from the sensor data
  val[0] = map(event.acceleration.x, -5, 5, 255, 5); //X
  val[1] = map(event.acceleration.y, -5, 5, 255, 5); //Y
  val[2] = map(event.acceleration.z, 5, 15, 200, 5); //Z

  //Smooth out the data points for each XYZ
  for (char i = 0; i < 3; ++i) {
    readingsTotal[i] -= readings[i][readIndex]; //Remove the last data point from the total
    readings[i][readIndex] = val[i]; //Insert the new data point into the array
    readingsTotal[i] += readings[i][readIndex]; //Add the new data point to the total
    val[i] = readingsTotal[i] / numReadings; //Calculate the average from the total
    val[i] = constrain(abs(val[i]), 5, 255); //Constrain the average to acceptable values
  }

  //Set all the Neopixel colours
  for (char i = 0; i < numPixels; ++i) {
    strip.setPixelColor(i, val[0], val[2], val[1]);
  }

  //Refresh the Neopixels
  strip.show();

  //Increment
  ++readIndex %= numReadings;
}

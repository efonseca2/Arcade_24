/*
  MPU6050 Raw

  A code for obtaining raw data from the MPU6050 module with the option to
  modify the data output format.

  Find the full MPU6050 library documentation here:
  https://github.com/ElectronicCats/mpu6050/wiki
*/
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <Bounce.h> 
#include "Adafruit_DRV2605.h"
#include <Keyboard.h>
/* MPU6050 default I2C address is 0x68*/
MPU6050 mpu;
//MPU6050 mpu(0x69);         //Use for AD0 high
//MPU6050 mpu(0x68, &Wire1); //Use for AD0 low, but 2nd Wire (TWI/I2C) object.
Adafruit_DRV2605 drv;

/* OUTPUT FORMAT DEFINITION----------------------------------------------------------------------------------
- Use "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated list of the accel 
X/Y/Z and gyro X/Y/Z values in decimal. Easy to read, but not so easy to parse, and slower over UART.

- Use "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit binary, one right after the other. 
As fast as possible without compression or data loss, easy to parse, but impossible to read for a human. 
This output format is used as an output.
--------------------------------------------------------------------------------------------------------------*/ 
#define OUTPUT_READABLE_ACCELGYRO
//#define OUTPUT_BINARY_ACCELGYRO

int16_t ax, ay, az;
int16_t gx, gy, gz;
bool blinkState;
Bounce button0 = Bounce(8, 10);
bool buzzOn = false;
unsigned long currentTime;
unsigned long prevTime = 0;

void setup() {
  /*--Start I2C interface--*/
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin(); 
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  Serial.begin(38400); //Initializate Serial wo work well at 8MHz/16MHz
  pinMode(8, INPUT_PULLUP);
  pinMode(6, OUTPUT);
  drv.begin();
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);

  /*Initialize device and check connection*/ 
  Serial.println("Initializing MPU...");
  mpu.initialize();
  Serial.println("Testing MPU6050 connection...");
  if(mpu.testConnection() ==  false){
    Serial.println("MPU6050 connection failed");
    while(true);
  }
  else{
    Serial.println("MPU6050 connection successful");
  }

  /* Use the code below to change accel/gyro offset values. Use MPU6050_Zero to obtain the recommended offsets */ 
  Serial.println("Updating internal sensor offsets...\n");
  mpu.setXAccelOffset(-3625); //Set your accelerometer offset for axis X
  mpu.setYAccelOffset(-46); //Set your accelerometer offset for axis Y
  mpu.setZAccelOffset(1406); //Set your accelerometer offset for axis Z
  mpu.setXGyroOffset(-62);  //Set your gyro offset for axis X
  mpu.setYGyroOffset(15);  //Set your gyro offset for axis Y
  mpu.setZGyroOffset(-15);  //Set your gyro offset for axis Z
  /*Print the defined offsets*/
  Serial.print("\t");
  Serial.print(mpu.getXAccelOffset());
  Serial.print("\t");
  Serial.print(mpu.getYAccelOffset()); 
  Serial.print("\t");
  Serial.print(mpu.getZAccelOffset());
  Serial.print("\t");
  Serial.print(mpu.getXGyroOffset()); 
  Serial.print("\t");
  Serial.print(mpu.getYGyroOffset());
  Serial.print("\t");
  Serial.print(mpu.getZGyroOffset());
  Serial.print("\n");

  /*Configure board LED pin for output*/ 
  pinMode(LED_BUILTIN, OUTPUT);
//eeeeeeeeeeeee
  buzzOn = true;
  prevTime = currentTime;
}

uint8_t effect = 14;

void loop() {
  /* Read raw accel/gyro data from the module. Other methods commented*/
  //mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  mpu.getAcceleration(&ax, &ay, &az);
  //mpu.getRotation(&gx, &gy, &gz);

  /*Print the obtained data on the defined format*/
  #ifdef OUTPUT_READABLE_ACCELGYRO
    //Serial.print("a/g:\t");
    Serial.print(ax); Serial.print("\t");
    Serial.println(ay); Serial.print("\t");
    //Serial.println(az); Serial.print("\t");
    //Serial.print(gx); Serial.print("\t");
    //Serial.print(gy); Serial.print("\t");
    //Serial.println(gz);
  #endif

  #ifdef OUTPUT_BINARY_ACCELGYRO
    Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
    Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
    Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
    Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
    Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
    Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
  #endif

  /*Blink LED to indicate activity*/
  // blinkState = !blinkState;
  // digitalWrite(LED_BUILTIN, blinkState);

  button0.update();

  currentTime = millis();

  if (button0.fallingEdge()) { // Button just pressed
    Keyboard.press(KEY_E);
    digitalWrite(6, HIGH); // Turn on the motor
    buzzOn = true;
    prevTime = currentTime; // Start the timer
    drv.setWaveform(0, effect); // Select the effect
    drv.setWaveform(1, 0); // End the waveform sequence
    drv.go(); // Start the effect
  }

  if (button0.risingEdge()) { // Button just released
    Keyboard.release(KEY_E);
  }

  if (buzzOn && (currentTime - prevTime >= 500)) {
    digitalWrite(6, LOW); // Turn off the motor
    buzzOn = false;
    drv.stop(); // Stop thee effect
  }

  int yValue;
  if (ay < 10000 && ay > -10000){
    yValue = map(ay, -10000, 10000, 0, 1023);
  } else if (ay <= -10000){
    yValue = 0;
  } 

    else if (ay >= 10000){
      yValue = 1023;
    }
  int xValue;
  if (ax < 10000 && ax > -10000){
    xValue = map(ax, -10000, 10000, 0, 1023);
  } else if (ax <= 10000){
    xValue = 0;
  }

  else if (ax >= 10000){
  xValue = 1023;
  }

  
   

  Joystick.Y(yValue);
  Joystick.X(xValue);
  



}

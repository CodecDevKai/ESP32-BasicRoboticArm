#include <Arduino.h>
// #include <Ps3Controller.h>
#include <PS4Controller.h>

#include <ESP32Servo.h>

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"

#define BASE_SERVO_PIN 18
#define SHOULDER_SERVO_PIN 19
#define ELBOW_SERVO_PIN 22
#define WRIST_SERVO_PIN 23

Servo BaseServo;
Servo ShoulderServo;
Servo ElbowServo;
Servo WristServo;

int leftJoystickX;
int leftJoystickY;

int rightJoystickX;
int rightJoystickY;

const int joystickDivision = 50;
const int joystickDeadzone = 2;

bool freezeServos = false;

int gyroX;
int gyroY;
int gyroZ;

int baseServoPos = 90;
int shoulderServoPos = 90;
int elbowServoPos = 90;
int wristServoPos = 90;

bool elbowLocked = false;

void notify() {

  gyroX = (PS4.GyrX());
  gyroY = (PS4.GyrY());
  gyroZ = (PS4.GyrZ());


  leftJoystickX = (PS4.LStickX());
  leftJoystickY = (PS4.LStickY());

  rightJoystickX = (PS4.RStickX());
  rightJoystickY = (PS4.RStickY());

  if(PS4.event.button_down.share) {
    Serial.println(PS4.L2Value());
  }

  if(PS4.event.button_down.ps) {
    ESP.restart();
  }

  if(PS4.event.button_down.options) {
    freezeServos = !freezeServos;
  }

  if(!freezeServos) {

    if(PS4.event.button_down.square) {
      elbowLocked = !elbowLocked;
    }
    
    if(PS4.event.button_down.l3) {
      baseServoPos = 90;
      BaseServo.write(baseServoPos);
      
      shoulderServoPos = 90;
      ShoulderServo.write(shoulderServoPos);

      delay(10);
    } 

    if(PS4.event.analog_move.stick.lx) {
      if(leftJoystickX < -2 && baseServoPos < 180) {
        
        baseServoPos -= floor(leftJoystickX/joystickDivision);
        BaseServo.write(baseServoPos);

      }
      if(leftJoystickX > 2 && baseServoPos > 0) {
        baseServoPos -= floor(leftJoystickX/joystickDivision);
        BaseServo.write(baseServoPos);

      }
    }

    if(PS4.event.analog_move.stick.ly) {
      if(leftJoystickY < -2 && shoulderServoPos < 180) {
        shoulderServoPos -= floor(leftJoystickY/joystickDivision);
        ShoulderServo.write(shoulderServoPos);

        if(elbowLocked){
          elbowServoPos += floor(leftJoystickY/joystickDivision);
          ElbowServo.write(elbowServoPos);
        }
      }
      if(leftJoystickY > 2 && shoulderServoPos > 0) {
        shoulderServoPos -= floor(leftJoystickY/joystickDivision);
        ShoulderServo.write(shoulderServoPos);
        
        if(elbowLocked){
          elbowServoPos += floor(leftJoystickY/joystickDivision);
          ElbowServo.write(elbowServoPos);
        }
      }
    }

    if(PS4.event.button_down.r3) {
      elbowServoPos = 90;
      ElbowServo.write(elbowServoPos);
      
      wristServoPos = 90;
      WristServo.write(wristServoPos);

      delay(10);
    } 

    if(PS4.event.analog_move.stick.rx) {
      if(rightJoystickX < -2 && wristServoPos < 180) {
        wristServoPos -= ceil(rightJoystickX/joystickDivision);
        WristServo.write(wristServoPos);
      }
      if(rightJoystickX > 2 && wristServoPos > 0) {
        wristServoPos -= ceil(rightJoystickX/joystickDivision);
        WristServo.write(wristServoPos);
      }
    }

    if(PS4.event.analog_move.stick.ry) {
      if(rightJoystickY < -2 && elbowServoPos < 180) {
        elbowServoPos -= floor(rightJoystickY/joystickDivision);
        ElbowServo.write(elbowServoPos);
      }
      if(rightJoystickY > 2 && elbowServoPos > 0) {
        elbowServoPos -= floor(rightJoystickY/joystickDivision);
        ElbowServo.write(elbowServoPos);
      }
    }
  }
}

float clip(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}

void onConnect() {
  Serial.println("Controller connected.");
  BaseServo.write(baseServoPos);
  ShoulderServo.write(shoulderServoPos);
  ElbowServo.write(elbowServoPos);
  WristServo.write(wristServoPos);
}

void onDisconnect() {
  Serial.println("Controller disconnected.");
}

void removePairedDevices() {
  uint8_t pairedDeviceBtAddr[20][6];
  int count = esp_bt_gap_get_bond_device_num();
  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
  for (int i = 0; i < count; i++) {
    esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
  }
}

void printDeviceAddress() {
  const uint8_t* point = esp_bt_dev_get_address();
  for (int i = 0; i < 6; i++) {
    char str[3];
    sprintf(str, "%02x", (int)point[i]);
    Serial.print(str);
    if(i < 5) {
      Serial.print(":");
    }
  }
}

void setup() 
{  
  Serial.begin(115200);
  Serial.println("Serial started.");

  BaseServo.attach(BASE_SERVO_PIN);
  ShoulderServo.attach(SHOULDER_SERVO_PIN);
  ElbowServo.attach(ELBOW_SERVO_PIN);
  WristServo.attach(WRIST_SERVO_PIN);

  PS4.attach(notify);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisconnect);
  PS4.begin();
  Serial.println("PS4 Controller Host set up at:");
  printDeviceAddress();
  


  Serial.println("Servos attached.");
}

// void setServosRandom() {
//   int pos = floor(rand() % 181);
//   baseServoPos = pos;
//   shoulderServoPos = pos;
//   elbowServoPos = pos;
//   wristServoPos = pos;

//   delay(500);
  
//   BaseServo.write(baseServoPos);
//   ShoulderServo.write(shoulderServoPos);
//   ElbowServo.write(elbowServoPos);
//   WristServo.write(wristServoPos);
// }

void loop() {
  if(!PS4.isConnected())
    Serial.println("No controllers found.");
    delay(2000);
    return;
  delay(2000);
}
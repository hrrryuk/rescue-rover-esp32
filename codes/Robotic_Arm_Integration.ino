/*
ResQ Rover - Robotic Arm Module

Hardware:

* ESP32 DevKit V1
* 4 x MG90S Servo Motors
* 5V 5A Buck Converter

Connections:

Base Servo      -> GPIO16
Shoulder Servo  -> GPIO17
Elbow Servo     -> GPIO19
Gripper Servo   -> GPIO15

Power:
Red   -> 5V
Brown -> GND
Orange -> Signal Pin
*/

#include <ESP32Servo.h>

Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;

#define BASE_SERVO      16
#define SHOULDER_SERVO  17
#define ELBOW_SERVO     19
#define GRIPPER_SERVO   15

void setup()
{
Serial.begin(115200);

baseServo.attach(BASE_SERVO);
shoulderServo.attach(SHOULDER_SERVO);
elbowServo.attach(ELBOW_SERVO);
gripperServo.attach(GRIPPER_SERVO);

Serial.println("Robotic Arm Initialized");
}

void loop()
{
// Base Rotation
baseServo.write(30);
delay(1000);

baseServo.write(90);
delay(1000);

baseServo.write(150);
delay(1000);

// Shoulder Movement
shoulderServo.write(60);
delay(1000);

shoulderServo.write(120);
delay(1000);

// Elbow Movement
elbowServo.write(60);
delay(1000);

elbowServo.write(120);
delay(1000);

// Gripper Movement
gripperServo.write(30);
delay(1000);

gripperServo.write(90);
delay(1000);

delay(2000);
}


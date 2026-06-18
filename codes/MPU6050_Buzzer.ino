/*
ResQ Rover - Fall Detection Module

Hardware:

* ESP32 DevKit V1
* MPU6050
* Active Buzzer

Connections:
MPU6050 SDA -> GPIO21
MPU6050 SCL -> GPIO22
MPU6050 VCC -> 3.3V
MPU6050 GND -> GND

Buzzer + -> GPIO23
Buzzer - -> GND
*/

#include <Wire.h>
#include <MPU6050_tockn.h>

#define BUZZER_PIN 23

MPU6050 mpu(Wire);

void setup()
{
Serial.begin(115200);

pinMode(BUZZER_PIN, OUTPUT);
digitalWrite(BUZZER_PIN, LOW);

Wire.begin(21, 22);

mpu.begin();

Serial.println("Calibrating MPU6050...");
delay(1000);

mpu.calcGyroOffsets(true);

Serial.println("System Ready");
}

void loop()
{
mpu.update();

float angleX = mpu.getAngleX();
float angleY = mpu.getAngleY();

static bool fallDetected = false;

if ((abs(angleX) > 45 || abs(angleY) > 45) && !fallDetected)
{
Serial.println("Fall Detected");

```
digitalWrite(BUZZER_PIN, HIGH);
delay(500);
digitalWrite(BUZZER_PIN, LOW);

fallDetected = true;
```

}

if (abs(angleX) < 20 && abs(angleY) < 20)
{
fallDetected = false;
}

delay(100);
}


/*
 Name:		MPUTest.ino
 Created:	1/20/2018 12:19:14 AM
 Author:	ToanND
*/
#include "Wire.h"
#define A_SCALE (16384.0)
const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float pitch, roll, yaw;

float dist(float a, float b)
{
	return sqrt((a*a) + (b*b));
}

void setup_mpu() {
	Wire.begin(D1, D2);
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x6B); // 0x6B: PWR_MGMT_1 register name (datasheet)
	Wire.write(0);
	Wire.endTransmission(true);
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	setup_mpu();
}

// the loop function runs over and over again until power down or reset
void loop() {
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x3B); //0x3B: ACCEL_XOUT_H register name (datasheet)
	Wire.endTransmission(false);
	Wire.requestFrom(MPU_addr, 14, 1);
	AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
	AcY = Wire.read() << 8 | Wire.read(); // 0x3D ... & 0x3E ...
	AcZ = Wire.read() << 8 | Wire.read(); //...
	Tmp = Wire.read() << 8 | Wire.read();
	GyX = Wire.read() << 8 | Wire.read();
	GyY = Wire.read() << 8 | Wire.read();
	GyZ = Wire.read() << 8 | Wire.read();
	if (AcX == -1 && AcY == -1 && AcZ == -1 && GyX == -1 && GyY == -1 && GyZ == -1)
		return;
	float radians1 = atan2(-AcX / A_SCALE, dist(AcY / A_SCALE, AcZ / A_SCALE));
	pitch = radians1 * 180 / 3.14;

	roll = atan2(AcY / A_SCALE, AcZ / A_SCALE) * 180 / 3.14;

	float radians3 = atan2(AcZ / A_SCALE, dist(AcX / A_SCALE, AcY / A_SCALE));
	yaw = radians3 * 180 / 3.14;
	printf("degrees: %f ----- %f ----- %f", pitch, roll, yaw);
	Serial.println(Tmp / 340.00 + 36.53);
}

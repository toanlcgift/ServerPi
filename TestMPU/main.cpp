#include <stdio.h>  
#include <wiringPiI2C.h>  
#include <wiringPi.h>  
#include "QMC5883L.h"
#include <math.h>

DFRobot_QMC5883 compass;

void loop() {
	Vector norm = compass.readNormalize();

	// Calculate heading
	float heading = atan2(norm.YAxis, norm.XAxis);

	// Set declination angle on your location and fix heading
	// You can find your declination on: http://magnetic-declination.com/
	// (+) Positive or (-) for negative
	// For Bytom / Poland declination angle is 4'26E (positive)
	// Formula: (deg + (min / 60.0)) / (180 / M_PI);
	float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / PI);
	heading += declinationAngle;

	// Correct for heading < 0deg and heading > 360deg
	if (heading < 0) {
		heading += 2 * PI;
	}

	if (heading > 2 * PI) {
		heading -= 2 * PI;
	}

	// Convert to degrees
	float headingDegrees = heading * 180 / M_PI;

	// Output
	printf("\n Heading = %f, Degrees = %f",heading, headingDegrees);

	delay(100);
}
int main()
{
	while (!compass.begin())
	{
		printf("Could not find a valid QMC5883 sensor, check wiring!");
		delay(500);
	}

	if (compass.isQMC()) {
		printf("Initialize QMC5883");
		compass.setRange(QMC5883_RANGE_2GA);
		compass.setMeasurementMode(QMC5883_CONTINOUS);
		compass.setDataRate(QMC5883_DATARATE_50HZ);
		compass.setSamples(QMC5883_SAMPLES_8);
	}
	while (true)
	{
		loop();
	}
	return 0;
}
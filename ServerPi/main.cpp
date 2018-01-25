#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include "QMC5883L.h"

#define MO1    0//index 11
#define MO2    2//index 13
#define MO3    3//index 15
#define MO4    12//index 19

#define TRIG (13) //index 21
#define ECHO (14) //index 23

#define SOCK_STREAM 1
#define AF_INET 2

int newsockfd;
int data;
DFRobot_QMC5883 compass;
float roll;
double distance;

pthread_t thread1, thread2, thread3, thread4;

void error(char *msg) {
	perror(msg);
	exit(1);
}

void checkRC(int rc, char *text) {
	if (rc < 0) {
		printf("Error: %s - %d\n");
		exit(-1);
	}
}

void sendData(int sockfd) {
	int n;
	char buffer[1000];
	memset(buffer, '\0', 1000);
	sprintf(buffer, "{\"distance\":\"%f\", \"roll\" : \"%.3f\"}\r\n", distance, roll);
	if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
		error(const_cast<char *>("ERROR writing to socket"));
}

int getData(int sockfd) {
	char buffer[1];
	int n;
	if ((n = read(sockfd, buffer, 1)) < 0)
		error(const_cast<char *>("ERROR reading from socket"));
	return buffer[0];
}

float dist(float a, float b)
{
	return sqrt((a*a) + (b*b));
}

void readQMC5883L() {
	Vector norm = compass.readNormalize();

	// Calculate heading
	float heading = atan2(norm.YAxis, norm.XAxis);

	// Set declination angle on your location and fix heading
	// You can find your declination on: http://magnetic-declination.com/
	// (+) Positive or (-) for negative
	// For HaNoi/VietNam declination angle is -1'28E (positive)
	// Formula: (deg + (min / 60.0)) / (180 / M_PI);
	float declinationAngle = (-1 + (28.0 / 60.0)) / (180 / M_PI);
	heading += declinationAngle;

	// Correct for heading < 0deg and heading > 360deg
	if (heading < 0) {
		heading += 2 * M_PI;
	}

	if (heading > 2 * M_PI) {
		heading -= 2 * M_PI;
	}

	// Convert to degrees
	roll = heading * 180 / M_PI;

	// Output
	//printf("\n Heading = %f, Degrees = %f", heading, roll);
}

void readHCSR04() {
	digitalWrite(TRIG, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG, LOW);
	unsigned int echoStart = millis();
	while (digitalRead(ECHO) == LOW && millis() - echoStart < 1000) {
		// do nothing
	}
	if (millis() - echoStart < 1000) {
		// Mark start
		unsigned int start = micros();
		while (digitalRead(ECHO) == HIGH) {
			// do nothing
		}
		// Mark end
		unsigned int end = micros();
		unsigned int delta = end - start;

		distance = 34029 * delta / 2000000.0;
		printf("Distance: %f\n", distance);
	}
}

void *readQMC5883Lloop(void *ptr) {
	while (1) {
		readQMC5883L();
	}
}

void *readHCSR04loop(void *ptr) {
	while (1) {
		readHCSR04();
	}
}

void *realtimeloop(void *ptr) {
	while (1) {
		sendData(newsockfd);
		delay(100);
	}
}

void *controlloop(void *ptr) {
	while (1) {
		data = getData(newsockfd);
		if (data < 0)
			break;
		else
			printf("got %d\n", data);
		switch (data)
		{
		case 'a':
			readQMC5883L();
			sendData(newsockfd);
			break;
		case 'b':
			readHCSR04();
			sendData(newsockfd);
			break;
		case 'c':
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, HIGH);
			break;
		case 'd':
			digitalWrite(MO1, HIGH);
			digitalWrite(MO2, LOW);
			break;
		case 'e':
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, LOW);
			break;
		case 'f':
			digitalWrite(MO3, HIGH);
			digitalWrite(MO4, LOW);
			break;
		case 'g':
			digitalWrite(MO3, LOW);
			digitalWrite(MO4, HIGH);
			break;
		case 'h':
			digitalWrite(MO3, LOW);
			digitalWrite(MO4, LOW);
			break;
		case 'i':
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, LOW);
			digitalWrite(MO3, LOW);
			digitalWrite(MO4, LOW);
			break;
		case 'j':
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, HIGH);
			digitalWrite(MO3, HIGH);
			digitalWrite(MO4, LOW);
			break;
		case 'k':
			digitalWrite(MO1, HIGH);
			digitalWrite(MO2, LOW);
			digitalWrite(MO3, LOW);
			digitalWrite(MO4, HIGH);
			break;
		default:
			break;
		}
	}
}

void initQMC5883L() {
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
}

int main(void)
{
	printf("Raspberry Pi\n");

	wiringPiSetupSys();
	wiringPiSetup();

	initQMC5883L();

	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);
	digitalWrite(TRIG, LOW);

	delay(50);

	pinMode(MO1, OUTPUT);
	pinMode(MO2, OUTPUT);
	pinMode(MO3, OUTPUT);
	pinMode(MO4, OUTPUT);
	digitalWrite(MO1, LOW);
	digitalWrite(MO2, LOW);
	digitalWrite(MO3, LOW);
	digitalWrite(MO4, LOW);

	int sockfd, portno = 51717, client;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	printf("using port #%d\n", portno);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(const_cast<char *>("ERROR opening socket"));
	bzero((char *)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
		sizeof(serv_addr)) < 0)
		error(const_cast<char *>("ERROR on binding"));
	listen(sockfd, 5);
	client = sizeof(cli_addr);
	//--- infinite wait on a connection ---
	while (1) {
		printf("waiting for new client...\n");
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&client)) < 0)
			error(const_cast<char *>("ERROR on accept"));
		printf("opened new communication with client %d\n", newsockfd);

		pthread_create(&thread1, NULL, readQMC5883Lloop, NULL);
		pthread_create(&thread2, NULL, readHCSR04loop, NULL);
		pthread_create(&thread3, NULL, realtimeloop, NULL);
		pthread_create(&thread4, NULL, controlloop, NULL);
		pthread_join(thread1, NULL);
		pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
		pthread_join(thread4, NULL);

		close(newsockfd);

		//--- if -2 sent by client, we can quit ---
		if (data == -2)
			break;
	}
	return 0;

}
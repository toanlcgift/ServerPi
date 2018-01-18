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

#define MO1    0//index 11
#define MO2    2//index 13
#define MO3    3//index 15
#define MO4    12//index 19

#define MPU6050_ADDRESS (0x68)
#define MPU6050_REG_PWR_MGMT_1 (0x6b)
#define MPU6050_REG_DATA_START (0x3b)
#define A_SCALE (16384.0)
#define ANG_SCALE (131.0)

#define TRIG (13) //index 21
#define ECHO (14) //index 23

#define SOCK_STREAM 1
#define AF_INET 2

int fd;
int newsockfd;
int data;

const int MPU_addr = 0x68;
short AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float pitch, roll, yaw;
double distance;

pthread_t thread1, thread2;

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
	sprintf(buffer, "{\"distance\":\"%f\", \"pitch\" : \"%.3f\", \"roll\" : \"%.3f\", \"yaw\" : \"%.3f\"}\r\n", distance, pitch, roll, yaw);
	if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
		error(const_cast<char *>("ERROR writing to socket"));
	delay(100);
	//buffer[n] = '\0';
}

int getData(int sockfd) {
	char buffer[1];
	int n;
	if ((n = read(sockfd, buffer, 1)) < 0)
		error(const_cast<char *>("ERROR reading from socket"));
	//buffer[n] = '\0';
	return buffer[0];
}

float dist(float a, float b)
{
	return sqrt((a*a) + (b*b));
}

void readMPU(int fd) {
	uint8_t msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START);
	uint8_t lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 1);
	AcX = (msb << 8) | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 2);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 3);
	AcY = (msb << 8) | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 4);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 5);
	AcZ = (msb << 8) | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 6);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 7);
	Tmp = (msb << 8) | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 8);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 9);
	GyX = (msb << 8) | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 10);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 11);
	GyY = (msb << 8) | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 12);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 13);
	GyZ = (msb << 8) | lsb;

	//printf("accelX=%f, accelY=%f, accelZ=%f, gyroX=%f, gyroY=%f, gyroZ=%f\n", AcX / A_SCALE, AcY / A_SCALE, AcZ / A_SCALE, GyX / ANG_SCALE, GyY / ANG_SCALE, GyZ / ANG_SCALE);
	float radians1 = atan2(-AcX / A_SCALE, dist(AcY / A_SCALE, AcZ / A_SCALE));
	pitch = radians1 * 180 / 3.14;

	roll = atan2(AcY/A_SCALE,AcZ/A_SCALE) * 180 / 3.14;

	float radians3 = atan2(AcZ / A_SCALE, dist(AcX / A_SCALE, AcY / A_SCALE));
	yaw = radians3 * 180 / 3.14;
	printf("degrees: %f ----- %f ----- %f", pitch, roll, yaw);
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

void *readclient(void *ptr) {

	while (1) {
		//---- wait for a number from client ---
		readHCSR04();
		readMPU(fd);
		printf("ptr: %d", newsockfd);
		sendData(newsockfd);
	}
}

void *writeclient(void *ptr) {
	while (1) {
		data = getData(newsockfd);
		if (data < 0)
			break;
		else
			printf("got %d\n", data);
		switch (data)
		{
		case 'a':
			readMPU(fd);
			sendData(newsockfd);
			//FIX ME: write MPU sensor data to client
			break;
		case 'b':
			readHCSR04();
			sendData(newsockfd);
			//FIX ME: write HCSR04 sensor data to client
			break;
		case 'c':
			printf("mo1 low mo2 high");
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, HIGH);
			break;
		case 'd':
			printf("mo1 high mo2 low");
			digitalWrite(MO1, HIGH);
			digitalWrite(MO2, LOW);
			break;
		case 'e':
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, LOW);
			break;
		case 'f':
			printf("mo3 high mo4 low");
			digitalWrite(MO3, HIGH);
			digitalWrite(MO4, LOW);
			break;
		case 'g':
			printf("mo3 low mo4 high");
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
		default:
			break;
		}
	}
}

int main(void)
{
	printf("Raspberry Pi\n");

	wiringPiSetupSys();
	wiringPiSetup();

	// Open an I2C connection
	fd = wiringPiI2CSetup(MPU6050_ADDRESS);
	checkRC(fd, "wiringPiI2CSetup");

	// Perform I2C work
	wiringPiI2CWriteReg8(fd, MPU6050_REG_PWR_MGMT_1, 0);

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

		int iret1 = pthread_create(&thread1, NULL, readclient, NULL);
		int iret2 = pthread_create(&thread2, NULL, writeclient, NULL);
		pthread_join(thread1, NULL);
		pthread_join(thread2, NULL);

		close(newsockfd);

		//--- if -2 sent by client, we can quit ---
		if (data == -2)
			break;
	}
	return 0;

}
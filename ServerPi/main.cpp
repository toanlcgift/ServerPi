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
#include <softPwm.h>

#define MO1    0//index 11
#define MO2    2//index 13
#define MO3    3//index 15
#define MO4    12//index 19

#define MPU6050_ADDRESS (0x68)
#define MPU6050_REG_PWR_MGMT_1 (0x6b)
#define MPU6050_REG_DATA_START (0x3b)
#define A_SCALE (16384.0)
#define ANG_SCALE (131.0)
#define PI 3.14

#define TRIG (13) //index 21
#define ECHO (14) //index 23

#define SOCK_STREAM 1
#define AF_INET 2

int fd;
int newsockfd;
int data;

int t = 0, dt = 1;
const int MPU_addr = 0x68;
short AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float Ax = 0, Ay = 0, Az = 0;
float Gx = 0, Gy = 0, Gz = 0;
float roll = 0, pitch = 0, rollgy = 0, pitchgy = 0, rollac = 0, pitchac = 0, gain = 0.95;
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
	sprintf(buffer, "{\"distance\":\"%f\", \"pitch\" : \"%.3f\", \"roll\" : \"%.3f\"}\r\n", distance, pitch, roll);
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

void readMPU(int fd) {

	t = millis();
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

	Ax = AcX / A_SCALE;
	Ay = AcY / A_SCALE;
	Az = AcZ / A_SCALE;

	Gx = GyX / ANG_SCALE;
	Gy = GyY / ANG_SCALE;
	Gz = GyZ / ANG_SCALE;

	printf("accelX=%f, accelY=%f, accelZ=%f, gyroX=%f, gyroY=%f, gyroZ=%f\n", Ax, Ay, Az, Gx, Gy, Gz);

	pitchgy = (Gy * ((float)dt / 1000)) + pitch;
	rollgy = (Gx * ((float)dt / 1000)) + roll;
	pitchac = atan2(Ax, Az) * (float)(180 / PI);
	rollac = atan2(Ay, Az) * (float)(180 / PI);
	roll = gain * rollgy + (1 - gain)* rollac;
	pitch = gain * pitchgy + (1 - gain) * pitchac;
	printf("roll: %f \n pitch: %f\n", roll, pitch);
	dt = millis() - t;
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

void *readMPUloop(void *ptr) {
	while (1) {
		readMPU(fd);
		if (pitch > 10 && pitch <= 45) {
			digitalWrite(MO1, HIGH);
			digitalWrite(MO2, LOW);
			digitalWrite(MO3, LOW);
			digitalWrite(MO4, HIGH);

		}
		else if (pitch < -10 && pitch >= -45) {
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, HIGH);
			digitalWrite(MO3, HIGH);
			digitalWrite(MO4, LOW);
		}
		else {
			digitalWrite(MO1, LOW);
			digitalWrite(MO2, LOW);
			digitalWrite(MO3, LOW);
			digitalWrite(MO4, LOW);
		}
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
			readMPU(fd);
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

	//softPwmCreate(MO1, OUTPUT, 50);

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

		pthread_create(&thread1, NULL, readMPUloop, NULL);
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
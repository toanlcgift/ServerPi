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

#define MO1    0//index 11
#define MO2    2//index 13
#define MO3    3//index 15
#define MO4    12//index 19

#define  MO5 15 //8
#define MO6 16 //10
#define MO7 1//12
#define MO8 4//16

#define MPU6050_ADDRESS (0x68)
#define MPU6050_REG_PWR_MGMT_1 (0x6b)
#define MPU6050_REG_DATA_START (0x3b)
#define A_SCALE (16384.0)
#define ANG_SCALE (131.0)

#define TRIG (13) //index 21
#define ECHO (14) //index 23

#define SOCK_STREAM 1
#define AF_INET 2

const int MPU_addr = 0x68;
short AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;



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

void sendData(int sockfd, int x) {
	int n;

	char buffer[32];
	sprintf(buffer, "%d\n", x);
	if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
		error(const_cast<char *>("ERROR writing to socket"));
	buffer[n] = '\0';
}

int getData(int sockfd) {
	char buffer[1];
	int n;
	if ((n = read(sockfd, buffer, 1)) < 0)
		error(const_cast<char *>("ERROR reading from socket"));
	//buffer[n] = '\0';
	return buffer[0];
}

void readMPU(int fd) {
	uint8_t msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START);
	uint8_t lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 1);
	AcX = msb << 8 | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 2);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 3);
	AcY = msb << 8 | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 4);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 5);
	AcZ = msb << 8 | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 6);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 7);
	Tmp = msb << 8 | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 8);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 9);
	GyX = msb << 8 | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 10);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 11);
	GyY = msb << 8 | lsb;

	msb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 12);
	lsb = wiringPiI2CReadReg8(fd, MPU6050_REG_DATA_START + 13);
	GyZ = msb << 8 | lsb;

	printf("accelX=%f, accelY=%f, accelZ=%f, gyroX=%f, gyroY=%f, gyroZ=%f\n", AcX / A_SCALE, AcY / A_SCALE, AcZ / A_SCALE, GyX / ANG_SCALE, GyY / ANG_SCALE, GyZ / ANG_SCALE);
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

		double distance = 34029 * delta / 2000000.0;
		printf("Distance: %f\n", distance);
	}
}

int main(void)
{
	printf("Raspberry Pi blink\n");

	wiringPiSetupSys();
	wiringPiSetup();

	// Open an I2C connection
	int fd = wiringPiI2CSetup(MPU6050_ADDRESS);
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

	int sockfd, newsockfd, portno = 51717, clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int data;

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
	clilen = sizeof(cli_addr);
	//--- infinite wait on a connection ---
	while (1) {
		printf("waiting for new client...\n");
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen)) < 0)
			error(const_cast<char *>("ERROR on accept"));
		printf("opened new communication with client\n");
		while (1) {
			//---- wait for a number from client ---
			data = getData(newsockfd);
			if (data < 0)
				break;
			else
				//FIX ME: write sensor data to client
				printf("got %d\n", data);
			switch (data)
			{
			case 'a':
				readMPU(fd);
				//FIX ME: write MPU sensor data to client
				break;
			case 'b':
				readHCSR04();
				//FIX ME: write HCSR04 sensor data to client
				break;
			case 'c':
				printf("mo1 high mo2 low");
				digitalWrite(MO1, HIGH);
				digitalWrite(MO2, LOW);
				break;
			case 'd':
				printf("mo1 low mo2 high");
				digitalWrite(MO1, LOW);
				digitalWrite(MO2, HIGH);
				break;
			case 'e':
				printf("mo3 high mo4 low");
				digitalWrite(MO3, HIGH);
				digitalWrite(MO4, LOW);
				break;
			case 'f':
				printf("mo3 low mo4 high");
				digitalWrite(MO3, LOW);
				digitalWrite(MO4, HIGH);
				break;
			case 'g':
				printf("mo5 high mo6 low");
				digitalWrite(MO5, HIGH);
				digitalWrite(MO6, LOW);
				break;
			case 'h':
				printf("mo5 low mo6 high");
				digitalWrite(MO5, LOW);
				digitalWrite(MO6, HIGH);
				break;
			case 'i':
				printf("mo7 high mo8 low");
				digitalWrite(MO7, HIGH);
				digitalWrite(MO8, LOW);
				break;
			case 'j':
				printf("mo7 low mo8 high");
				digitalWrite(MO7, LOW);
				digitalWrite(MO8, HIGH);
				break;
			case 'k':
				digitalWrite(MO1, LOW);
				digitalWrite(MO2, LOW);
				digitalWrite(MO3, LOW);
				digitalWrite(MO4, LOW);
				digitalWrite(MO5, LOW);
				digitalWrite(MO6, LOW);
				digitalWrite(MO7, LOW);
				digitalWrite(MO8, LOW);
				break;
			default:
				break;
			}
		}
		close(newsockfd);

		//--- if -2 sent by client, we can quit ---
		if (data == -2)
			break;
	}
	return 0;

}
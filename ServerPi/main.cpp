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

#define GPIO0     11
#define GPIO2    13
#define MPU6050_ADDRESS (0x68)
#define MPU6050_REG_PWR_MGMT_1 (0x6b)
#define MPU6050_REG_DATA_START (0x3b)
#define A_SCALE (16384.0)
#define ANG_SCALE (131.0)
#define SOCK_STREAM 1
#define AF_INET 2

const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;


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

	pinMode(GPIO0, OUTPUT);
	pinMode(GPIO2, OUTPUT);

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
				printf("got %d\n", data);
			if (data == 49) {
				readMPU(fd);
			}
			else if (data == 2) {
				digitalWrite(GPIO0, HIGH);
				digitalWrite(GPIO2, HIGH);
			}
			else if (data == 3) {
				digitalWrite(GPIO2, HIGH);
			}
			else if (data == 4) {
				digitalWrite(GPIO0, LOW);   // Off
				digitalWrite(GPIO2, LOW);
			}
		}
		close(newsockfd);

		//--- if -2 sent by client, we can quit ---
		if (data == -2)
			break;
	}
	return 0;

}
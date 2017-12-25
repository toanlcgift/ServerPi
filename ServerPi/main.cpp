#include <wiringPi.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
// we have to use BCM numbering when initializing with wiringPiSetupSys
// when choosing a different pin number please use the BCM numbering, also
// update the Property Pages - Build Events - Remote Post-Build Event command 
// which uses gpio export for setup for wiringPiSetupSys
#define GPIO0     11
#define GPIO2    13
#define SOCK_STREAM 1
#define AF_INET 2
void error(char *msg) {
	perror(msg);
	exit(1);
}

int func(int a) {
	return 2 * a;
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

int main(void)
{
	printf("Raspberry Pi blink\n");

	wiringPiSetupSys();
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
			if (data == 1) {
				digitalWrite(GPIO0, HIGH);
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
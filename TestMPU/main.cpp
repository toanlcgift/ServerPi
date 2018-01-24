#include <stdio.h>  
#include <wiringPiI2C.h>  
#include <wiringPi.h>  
int main()
{
	int fd_Magnetic = wiringPiI2CSetup(0x0d);
	if (fd_Magnetic < 0)
	{
		printf("Error Setup wiringPi I2C!\n");
	}
	wiringPiI2CWriteReg8(fd_Magnetic, 0x09, 0x1d);
	while (1)
	{
		printf("%x ", wiringPiI2CReadReg8(fd_Magnetic, 0x00));
		printf("%x ", wiringPiI2CReadReg8(fd_Magnetic, 0x01));
		printf("%x ", wiringPiI2CReadReg8(fd_Magnetic, 0x02));
		printf("%x ", wiringPiI2CReadReg8(fd_Magnetic, 0x03));
		printf("%x ", wiringPiI2CReadReg8(fd_Magnetic, 0x04));
		printf("%x \n", wiringPiI2CReadReg8(fd_Magnetic, 0x05));
		delay(500);
	}
	return 0;
}
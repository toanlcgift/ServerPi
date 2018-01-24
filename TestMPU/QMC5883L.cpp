
#include "QMC5883L.h"
#include <stdio.h>  
#include <wiringPi.h>
#include <wiringPiI2C.h>

int fd;

bool DFRobot_QMC5883::begin()
{
	int retry;
	retry = 5;
	isQMC_ = true;
	{
		retry = 5;
		while (retry--) {
			fd = wiringPiI2CSetup(QMC5883_ADDRESS);
			if (fd < 0)
			{
				printf("Error Setup wiringPi I2C!\n");
			}
		}
		writeRegister8(QMC5883_REG_IDENT_B, 0X01);
		writeRegister8(QMC5883_REG_IDENT_C, 0X40);
		writeRegister8(QMC5883_REG_IDENT_D, 0X01);
		writeRegister8(QMC5883_REG_CONFIG_1, 0X1D);
		auto x = fastRegister8(QMC5883_REG_IDENT_B);
		auto y = fastRegister8(QMC5883_REG_IDENT_C);
		auto z = fastRegister8(QMC5883_REG_IDENT_D);

		//printf("this is x, y ,z: %x %x %x", x, y, z);
		if (x != 0x01
			|| y != 0x40
			|| z != 0x01) {
			return false;
		}
		setRange(QMC5883_RANGE_8GA);
		setMeasurementMode(QMC5883_CONTINOUS);
		setDataRate(QMC5883_DATARATE_50HZ);
		setSamples(QMC5883_SAMPLES_8);
		mgPerDigit = 4.35f;
		return true;
	}
	return false;
}

Vector DFRobot_QMC5883::readRaw(void)
{
	int range = 10;
	float Xsum = 0.0;
	float Ysum = 0.0;
	float Zsum = 0.0;
	if (isQMC_) {
		while (range--) {
			v.XAxis = readRegister16(QMC5883_REG_OUT_X_M);
			v.YAxis = readRegister16(QMC5883_REG_OUT_Y_M);
			v.ZAxis = readRegister16(QMC5883_REG_OUT_Z_M);
			calibrate();
			Xsum += v.XAxis;
			Ysum += v.YAxis;
			Zsum += v.ZAxis;
		}
		v.XAxis = Xsum / range;
		v.YAxis = Ysum / range;
		v.ZAxis = Zsum / range;
		if (firstRun) {
			initMinMax();
			firstRun = false;
		}
	}
	return v;
}
void DFRobot_QMC5883::calibrate()
{
	if (v.XAxis < minX) minX = v.XAxis;
	if (v.XAxis > maxX) maxX = v.XAxis;
	if (v.YAxis < minY) minY = v.YAxis;
	if (v.YAxis > maxY) maxY = v.YAxis;
	if (v.ZAxis < minZ) minZ = v.ZAxis;
	if (v.ZAxis > maxZ) maxZ = v.ZAxis;
}
void DFRobot_QMC5883::initMinMax()
{
	minX = v.XAxis;
	maxX = v.XAxis;
	minY = v.YAxis;
	maxY = v.YAxis;
	minZ = v.ZAxis;
	maxZ = v.ZAxis;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

Vector DFRobot_QMC5883::readNormalize(void)
{
	int range = 10;
	float Xsum = 0.0;
	float Ysum = 0.0;
	float Zsum = 0.0;
	if (isQMC_) {
		while (range--) {
			v.XAxis = ((float)readRegister16(QMC5883_REG_OUT_X_M)) * mgPerDigit;
			v.YAxis = ((float)readRegister16(QMC5883_REG_OUT_Y_M)) * mgPerDigit;
			v.ZAxis = (float)readRegister16(QMC5883_REG_OUT_Z_M) * mgPerDigit;
			Xsum += v.XAxis;
			Ysum += v.YAxis;
			Zsum += v.ZAxis;
		}
		v.XAxis = Xsum / range;
		v.YAxis = Ysum / range;
		v.ZAxis = Zsum / range;
		if (firstRun) {
			initMinMax();
			firstRun = false;
		}

		calibrate();
		v.XAxis = map(v.XAxis, minX, maxX, -360, 360);
		v.YAxis = map(v.YAxis, minY, maxY, -360, 360);
		v.ZAxis = map(v.ZAxis, minZ, maxZ, -360, 360);
	}
	return v;
}

void DFRobot_QMC5883::setRange(QMC5883_range_t range)
{
	if (isQMC_) {
		switch (range)
		{
		case QMC5883_RANGE_2GA:
			mgPerDigit = 1.22f;
			break;
		case QMC5883_RANGE_8GA:
			mgPerDigit = 4.35f;
			break;
		default:
			break;
		}

		writeRegister8(QMC5883_REG_CONFIG_2, range << 4);
	}
}

QMC5883_range_t DFRobot_QMC5883::getRange(void)
{
	if (isQMC_) {
		return (QMC5883_range_t)((readRegister8(QMC5883_REG_CONFIG_2) >> 4));
	}
	return QMC5883_RANGE_8GA;
}

void DFRobot_QMC5883::setMeasurementMode(QMC5883_mode_t mode)
{
	int value;
	if (isQMC_) {
		value = readRegister8(QMC5883_REG_CONFIG_1);
		value &= 0xfc;
		value |= mode;

		writeRegister8(QMC5883_REG_CONFIG_1, value);
	}
}

QMC5883_mode_t DFRobot_QMC5883::getMeasurementMode(void)
{
	int value = 0;
	if (isQMC_) {
		value = readRegister8(QMC5883_REG_CONFIG_1);
	}
	value &= 0b00000011;
	return (QMC5883_mode_t)value;
}

void DFRobot_QMC5883::setDataRate(QMC5883_dataRate_t dataRate)
{
	int value;
	if (isQMC_) {
		value = readRegister8(QMC5883_REG_CONFIG_1);
		value &= 0xf3;
		value |= (dataRate << 2);

		writeRegister8(QMC5883_REG_CONFIG_1, value);
	}
}

QMC5883_dataRate_t DFRobot_QMC5883::getDataRate(void)
{
	int value = 0;
	if (isQMC_) {
		value = readRegister8(QMC5883_REG_CONFIG_1);
		value &= 0b00001100;
		value >>= 2;
	}
	return (QMC5883_dataRate_t)value;
}

void DFRobot_QMC5883::setSamples(QMC5883_samples_t samples)
{
	int value;
	if (isQMC_) {
		value = readRegister8(QMC5883_REG_CONFIG_1);
		value &= 0x3f;
		value |= (samples << 6);
		writeRegister8(QMC5883_REG_CONFIG_1, value);
	}
}

QMC5883_samples_t DFRobot_QMC5883::getSamples(void)
{
	int value = 0;
	if (isQMC_) {
		value = readRegister8(QMC5883_REG_CONFIG_1);
		value &= 0x3f;
		value >>= 6;
	}
	return (QMC5883_samples_t)value;
}

// Write byte to register
void DFRobot_QMC5883::writeRegister8(int reg, int value)
{
	wiringPiI2CWriteReg8(fd, reg, value);
}
// Read byte to register
int DFRobot_QMC5883::fastRegister8(int reg)
{
	wiringPiI2CWrite(fd, reg);
	return wiringPiI2CRead(fd);
}

// Read byte from register
int DFRobot_QMC5883::readRegister8(int reg)
{
	return wiringPiI2CReadReg8(fd, reg);
}
// Read word from register
int DFRobot_QMC5883::readRegister16(int reg)
{
	return wiringPiI2CReadReg16(fd, reg);
}

int DFRobot_QMC5883::getICType(void)
{
	if (isQMC_) {
		return IC_QMC5883;
	}
	else {
		return IC_NONE;
	}
}

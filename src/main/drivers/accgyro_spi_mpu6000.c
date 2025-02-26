/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * Authors:
 * Dominic Clifton - Cleanflight implementation
 * John Ihlein - Initial FF32 code
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "common/axis.h"
#include "common/maths.h"

#include "system.h"
#include "gpio.h"
#include "bus_spi.h"

#include "sensor.h"
#include "accgyro.h"
#include "accgyro_spi_mpu6000.h"

#include "gyro_sync.h"

static bool mpuSpi6000InitDone = false;

// Registers
#define MPU6000_PRODUCT_ID      	0x0C
#define MPU6000_SMPLRT_DIV	    	0x19
#define MPU6000_GYRO_CONFIG	    	0x1B
#define MPU6000_ACCEL_CONFIG  		0x1C
#define MPU6000_FIFO_EN		    	0x23
#define MPU6000_INT_PIN_CFG	    	0x37
#define MPU6000_INT_ENABLE	    	0x38
#define MPU6000_INT_STATUS	    	0x3A
#define MPU6000_ACCEL_XOUT_H 		0x3B
#define MPU6000_ACCEL_XOUT_L 		0x3C
#define MPU6000_ACCEL_YOUT_H 		0x3D
#define MPU6000_ACCEL_YOUT_L 		0x3E
#define MPU6000_ACCEL_ZOUT_H 		0x3F
#define MPU6000_ACCEL_ZOUT_L    	0x40
#define MPU6000_TEMP_OUT_H	    	0x41
#define MPU6000_TEMP_OUT_L	    	0x42
#define MPU6000_GYRO_XOUT_H	    	0x43
#define MPU6000_GYRO_XOUT_L	    	0x44
#define MPU6000_GYRO_YOUT_H	    	0x45
#define MPU6000_GYRO_YOUT_L	     	0x46
#define MPU6000_GYRO_ZOUT_H	    	0x47
#define MPU6000_GYRO_ZOUT_L	    	0x48
#define MPU6000_USER_CTRL	    	0x6A
#define MPU6000_SIGNAL_PATH_RESET   0x68
#define MPU6000_PWR_MGMT_1	    	0x6B
#define MPU6000_PWR_MGMT_2	    	0x6C
#define MPU6000_FIFO_COUNTH	    	0x72
#define MPU6000_FIFO_COUNTL	    	0x73
#define MPU6000_FIFO_R_W		   	0x74
#define MPU6000_WHOAMI		    	0x75

// Bits
#define BIT_SLEEP				    0x40
#define BIT_H_RESET				    0x80
#define BITS_CLKSEL				    0x07
#define MPU_CLK_SEL_PLLGYROX	    0x01
#define MPU_CLK_SEL_PLLGYROZ	    0x03
#define MPU_EXT_SYNC_GYROX		    0x02
#define BITS_FS_250DPS              0x00
#define BITS_FS_500DPS              0x08
#define BITS_FS_1000DPS             0x10
#define BITS_FS_2000DPS             0x18
#define BITS_FS_2G                  0x00
#define BITS_FS_4G                  0x08
#define BITS_FS_8G                  0x10
#define BITS_FS_16G                 0x18
#define BITS_FS_MASK                0x18
#define BITS_DLPF_CFG_256HZ         0x00
#define BITS_DLPF_CFG_188HZ         0x01
#define BITS_DLPF_CFG_98HZ          0x02
#define BITS_DLPF_CFG_42HZ          0x03
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06
#define BITS_DLPF_CFG_2100HZ_NOLPF  0x07
#define BITS_DLPF_CFG_MASK          0x07
#define BIT_INT_ANYRD_2CLEAR        0x10
#define BIT_RAW_RDY_EN			    0x01
#define BIT_I2C_IF_DIS              0x10
#define BIT_INT_STATUS_DATA		    0x01
#define BIT_GYRO                    3
#define BIT_ACC                     2
#define BIT_TEMP                    1

// Product ID Description for MPU6000
// high 4 bits low 4 bits
// Product Name Product Revision
#define MPU6000ES_REV_C4 0x14
#define MPU6000ES_REV_C5 0x15
#define MPU6000ES_REV_D6 0x16
#define MPU6000ES_REV_D7 0x17
#define MPU6000ES_REV_D8 0x18
#define MPU6000_REV_C4 0x54
#define MPU6000_REV_C5 0x55
#define MPU6000_REV_D6 0x56
#define MPU6000_REV_D7 0x57
#define MPU6000_REV_D8 0x58
#define MPU6000_REV_D9 0x59
#define MPU6000_REV_D10 0x5A

#define DISABLE_MPU6000       GPIO_SetBits(MPU6000_CS_GPIO,   MPU6000_CS_PIN)
#define ENABLE_MPU6000        GPIO_ResetBits(MPU6000_CS_GPIO, MPU6000_CS_PIN)

bool mpu6000SpiGyroRead(int16_t *gyroADC);
bool mpu6000SpiAccRead(int16_t *gyroADC);
void checkMPU6000Interrupt(bool *gyroIsUpdated);

static void mpu6000WriteRegister(uint8_t reg, uint8_t data)
{
    ENABLE_MPU6000;
    spiTransferByte(MPU6000_SPI_INSTANCE, reg);
    spiTransferByte(MPU6000_SPI_INSTANCE, data);
    DISABLE_MPU6000;
}

static void mpu6000ReadRegister(uint8_t reg, uint8_t *data, int length)
{
    ENABLE_MPU6000;
    spiTransferByte(MPU6000_SPI_INSTANCE, reg | 0x80); // read transaction
    spiTransfer(MPU6000_SPI_INSTANCE, data, NULL, length);
    DISABLE_MPU6000;
}

void mpu6000SpiGyroInit(void)
{
}

void mpu6000SpiAccInit(void)
{
    acc_1G = 512 * 8;
}

bool mpu6000SpiDetect(void)
{
    uint8_t in;
    uint8_t attemptsRemaining = 5;
    if (mpuSpi6000InitDone) {
        return true;
    }

#ifdef STM32F40_41xxx
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_0_65625MHZ_CLOCK_DIVIDER);
#else
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_0_5625MHZ_CLOCK_DIVIDER);
#endif

    mpu6000WriteRegister(MPU6000_PWR_MGMT_1, BIT_H_RESET);
    return true;

    do {
        delay(150);

        mpu6000ReadRegister(MPU6000_WHOAMI, &in, 1);
        if (in == MPU6000_WHO_AM_I_CONST) {
            break;
        }
        if (!attemptsRemaining) {
            return false;
        }
    } while (attemptsRemaining--);


    mpu6000ReadRegister(MPU6000_PRODUCT_ID, &in, 1);

    /* look for a product ID we recognise */

    // verify product revision
    switch (in) {
        case MPU6000ES_REV_C4:
        case MPU6000ES_REV_C5:
        case MPU6000_REV_C4:
        case MPU6000_REV_C5:
        case MPU6000ES_REV_D6:
        case MPU6000ES_REV_D7:
        case MPU6000ES_REV_D8:
        case MPU6000_REV_D6:
        case MPU6000_REV_D7:
        case MPU6000_REV_D8:
        case MPU6000_REV_D9:
        case MPU6000_REV_D10:
            return true;
    }

    return false;
}

void mpu6000AccAndGyroInit(void) {

    if (mpuSpi6000InitDone) {
        return;
    }

#ifdef STM32F40_41xxx
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_0_65625MHZ_CLOCK_DIVIDER);
#else
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_0_5625MHZ_CLOCK_DIVIDER);
#endif

    // Device Reset
    mpu6000WriteRegister(MPU6000_PWR_MGMT_1, BIT_H_RESET);
    delay(150);

    mpu6000WriteRegister(MPU6000_SIGNAL_PATH_RESET, BIT_GYRO | BIT_ACC | BIT_TEMP);
    delay(150);

    // Clock Source PPL with Z axis gyro reference
    mpu6000WriteRegister(MPU6000_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
    delayMicroseconds(1);

    // Disable Primary I2C Interface
    mpu6000WriteRegister(MPU6000_USER_CTRL, BIT_I2C_IF_DIS);
    delayMicroseconds(1);

    mpu6000WriteRegister(MPU6000_PWR_MGMT_2, 0x00);
    delayMicroseconds(1);

    // Accel Sample Rate 1kHz
    // Gyroscope Output Rate =  1kHz when the DLPF is enabled
    mpu6000WriteRegister(MPU6000_SMPLRT_DIV, 0x00);
    delayMicroseconds(1);

    // Accel +/- 8 G Full Scale
    mpu6000WriteRegister(MPU6000_ACCEL_CONFIG, BITS_FS_8G);
    delayMicroseconds(1);

    // Gyro +/- 1000 DPS Full Scale
    mpu6000WriteRegister(MPU6000_GYRO_CONFIG, BITS_FS_2000DPS);
    delayMicroseconds(1);

    mpuSpi6000InitDone = true;
}

bool mpu6000SpiAccDetect(acc_t *acc)
{
    if (!mpu6000SpiDetect()) {
        return false;
    }

    spiResetErrorCounter(MPU6000_SPI_INSTANCE);

    mpu6000AccAndGyroInit();

    acc->init = mpu6000SpiAccInit;
    acc->read = mpu6000SpiAccRead;

    delay(100);
    return true;
}

bool mpu6000SpiGyroDetect(gyro_t *gyro, uint16_t lpf)
{
    if (!mpu6000SpiDetect()) {
        return false;
    }

    spiResetErrorCounter(MPU6000_SPI_INSTANCE);

    mpu6000AccAndGyroInit();

    uint8_t mpuLowPassFilter = BITS_DLPF_CFG_42HZ;
    int16_t data[3];

    // default lpf is 42Hz
    if (lpf == 256)
        mpuLowPassFilter = BITS_DLPF_CFG_256HZ;
    else if (lpf >= 188)
        mpuLowPassFilter = BITS_DLPF_CFG_188HZ;
    else if (lpf >= 98)
        mpuLowPassFilter = BITS_DLPF_CFG_98HZ;
    else if (lpf >= 42)
        mpuLowPassFilter = BITS_DLPF_CFG_42HZ;
    else if (lpf >= 20)
        mpuLowPassFilter = BITS_DLPF_CFG_20HZ;
    else if (lpf >= 10)
        mpuLowPassFilter = BITS_DLPF_CFG_10HZ;
    else if (lpf > 0)
        mpuLowPassFilter = BITS_DLPF_CFG_5HZ;
    else
        mpuLowPassFilter = BITS_DLPF_CFG_256HZ;

#ifdef STM32F40_41xxx
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_0_65625MHZ_CLOCK_DIVIDER);
#else
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_0_5625MHZ_CLOCK_DIVIDER);
#endif

    // Determine the new sample divider
    mpu6000WriteRegister(MPU6000_SMPLRT_DIV, gyroMPU6xxxGetDivider());
    delayMicroseconds(1);

    // Accel and Gyro DLPF Setting
    mpu6000WriteRegister(MPU6000_CONFIG, mpuLowPassFilter);
    delayMicroseconds(1);

    mpu6000SpiGyroRead(data);

    if ((((int8_t)data[1]) == -1 && ((int8_t)data[0]) == -1) || spiGetErrorCounter(MPU6000_SPI_INSTANCE) != 0) {
        spiResetErrorCounter(MPU6000_SPI_INSTANCE);
        return false;
    }
    gyro->init = mpu6000SpiGyroInit;
    gyro->read = mpu6000SpiGyroRead;
    gyro->intStatus = checkMPU6000Interrupt;
    // 16.4 dps/lsb scalefactor
    gyro->scale = 1.0f / 16.4f;
    //gyro->scale = (4.0f / 16.4f) * (M_PIf / 180.0f) * 0.000001f;
    delay(100);
    return true;
}

bool mpu6000SpiGyroRead(int16_t *gyroData)
{
    uint8_t buf[6];

#ifdef STM32F40_41xxx
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_21MHZ_CLOCK_DIVIDER);
#else
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_18MHZ_CLOCK_DIVIDER);  // 18 MHz SPI clock
#endif

    mpu6000ReadRegister(MPU6000_GYRO_XOUT_H, buf, 6);

    gyroData[X] = (int16_t)((buf[0] << 8) | buf[1]);
    gyroData[Y] = (int16_t)((buf[2] << 8) | buf[3]);
    gyroData[Z] = (int16_t)((buf[4] << 8) | buf[5]);

    return true;
}

bool mpu6000SpiAccRead(int16_t *gyroData)
{
    uint8_t buf[6];

#ifdef STM32F40_41xxx
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_21MHZ_CLOCK_DIVIDER);
#else
    spiSetDivisor(MPU6000_SPI_INSTANCE, SPI_18MHZ_CLOCK_DIVIDER);  // 18 MHz SPI clock
#endif

    mpu6000ReadRegister(MPU6000_ACCEL_XOUT_H, buf, 6);

    gyroData[X] = (int16_t)((buf[0] << 8) | buf[1]);
    gyroData[Y] = (int16_t)((buf[2] << 8) | buf[3]);
    gyroData[Z] = (int16_t)((buf[4] << 8) | buf[5]);

    return true;
}

void checkMPU6000Interrupt(bool *gyroIsUpdated) {
	uint8_t mpuIntStatus;

	mpu6000ReadRegister(MPU6000_INT_STATUS, &mpuIntStatus, 1);

	delayMicroseconds(5);

	(mpuIntStatus) ? (*gyroIsUpdated= true) : (*gyroIsUpdated= false);
}

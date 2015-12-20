
#pragma once

#define GYRO_SCALE_FACTOR  0.00053292f  // (4/131) * pi/180   (32.75 LSB = 1 DPS)

#define MPU6000_WHO_AM_I_CONST              (0x68)
#define MPU9250_WHO_AM_I_CONST              (0x71)

// RF = Register Flag
#define MPU_RF_DATA_RDY_EN (1 << 0)

bool mpu6000SpiDetect(void);

bool mpu6000SpiAccDetect(acc_t *acc);
bool mpu6000SpiGyroDetect(gyro_t *gyro);

bool mpu6000WriteRegister(uint8_t reg, uint8_t data);
bool mpu6000ReadRegister(uint8_t reg, uint8_t length, uint8_t *data);

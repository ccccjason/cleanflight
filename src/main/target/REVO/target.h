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

#pragma once
#define TARGET_BOARD_IDENTIFIER "REVO"

#define LED0_GPIO   GPIOB
#define LED0_PIN    Pin_5 // Blue LEDs - PB5
#define LED0_PERIPHERAL RCC_AHB1Periph_GPIOB
#define LED1_GPIO   GPIOB
#define LED1_PIN    Pin_4  // Orange LEDs - PB4
#define LED1_PERIPHERAL RCC_AHB1Periph_GPIOB

#define INVERTER_PIN Pin_0 // PC0 used as inverter select GPIO
#define INVERTER_GPIO GPIOC
#define INVERTER_PERIPHERAL RCC_AHB1Periph_GPIOC
#define INVERTER_USART USART1

#define MPU6000_CS_GPIO       GPIOA
#define MPU6000_CS_PIN        GPIO_Pin_4
#define MPU6000_SPI_INSTANCE  SPI1

#define ACC
#define USE_ACC_SPI_MPU6000
#define ACC_SPI_MPU6000_ALIGN CW270_DEG

#define GYRO
#define USE_GYRO_SPI_MPU6000
#define GYRO_SPI_MPU6000_ALIGN CW270_DEG

#define INVERTER
#define LED0
#define LED1

#define USE_USART1

#define USE_USART3
#define USART3_RX_PIN Pin_11
#define USART3_TX_PIN Pin_10
#define USART3_GPIO GPIOB
#define USART3_APB1_PERIPHERALS RCC_APB1Periph_USART3
#define USART3_AHB1_PERIPHERALS RCC_AHB1Periph_GPIOB

#define SERIAL_PORT_COUNT 2

#define USE_SPI
#define USE_SPI_DEVICE_1
#define USE_SPI_DEVICE_2

#define USE_I2C
#define I2C_DEVICE_INT (I2CDEV_1)

#define SENSORS_SET (SENSOR_ACC)

#define LED_STRIP
#define LED_STRIP_TIMER TIM5
#define SERIAL_RX
#define AUTOTUNE

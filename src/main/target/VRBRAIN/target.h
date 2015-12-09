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
#define TARGET_BOARD_IDENTIFIER "VRBRAIN"

#define LED0_GPIO   GPIOD
#define LED0_PIN    Pin_14 // Blue LEDs - PB5
#define LED0_PERIPHERAL RCC_AHB1Periph_GPIOD
#define LED1_GPIO   GPIOD
#define LED1_PIN    Pin_15  // Red LEDs - PB4
#define LED1_PERIPHERAL RCC_AHB1Periph_GPIOD

#define BEEP_GPIO   GPIOA
#define BEEP_PIN    Pin_0
#define BEEP_PERIPHERAL RCC_AHB1Periph_GPIOA
//#define BEEPER_INVERTED


#define MPU6000_CS_GPIO       GPIOE
#define MPU6000_CS_PIN        GPIO_Pin_10
#define MPU6000_SPI_INSTANCE  SPI2

#define GYRO
#define USE_GYRO_SPI_MPU6000
#define GYRO_MPU6000_ALIGN CW270_DEG

#define ACC
#define USE_ACC_SPI_MPU6000
#define ACC_MPU6000_ALIGN CW270_DEG


// MPU6000 interrupts
#define USE_MPU_DATA_READY_SIGNAL
//#define ENSURE_MPU_DATA_READY_IS_LOW

#define EXTI15_10_CALLBACK_HANDLER_COUNT 1

/*
#define MAG
//#define USE_MAG_HMC5883
#define HMC5883_BUS I2C_DEVICE_INT
#define MAG_HMC5883_ALIGN CW90_DEG

#define BARO
#define USE_BARO_MS5611
#define MS5611_BUS I2C_DEVICE_INT
*/

//#define INVERTER
#define LED0
#define LED1
#define BEEPER

/*
#define M25P16_CS_GPIO        GPIOE
#define M25P16_CS_PIN         GPIO_Pin_15
#define M25P16_SPI_INSTANCE   SPI3
*/

/*
#define USE_FLASHFS
#define USE_FLASH_M25P16
*/

#define USABLE_TIMER_CHANNEL_COUNT 12

#define USE_VCP

#define USE_USART1
#define USART1_RX_PIN Pin_7
#define USART1_TX_PIN Pin_6
#define USART1_GPIO GPIOB
#define USART1_APB2_PERIPHERALS RCC_APB2Periph_USART1
#define USART1_AHB1_PERIPHERALS RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_DMA2

#define USE_USART3
#define USART3_RX_PIN Pin_9
#define USART3_TX_PIN Pin_8
#define USART3_GPIO GPIOD
#define USART3_APB1_PERIPHERALS RCC_APB1Periph_USART3
#define USART3_AHB1_PERIPHERALS RCC_AHB1Periph_GPIOD

#define USE_USART6
#define USART6_RX_PIN Pin_7
#define USART6_TX_PIN Pin_6
#define USART6_GPIO GPIOC
#define USART6_APB2_PERIPHERALS RCC_APB2Periph_USART6
#define USART6_AHB1_PERIPHERALS RCC_AHB1Periph_GPIOC

/*
#define INVERTER_PIN Pin_0 // PC0 used as inverter select GPIO
#define INVERTER_GPIO GPIOC
#define INVERTER_PERIPHERAL RCC_AHB1Periph_GPIOC
#define INVERTER_USART USART1
*/

#define SERIAL_PORT_COUNT 4


#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_HARDWARE 0 // PWM 1

#define USE_SPI
#define USE_SPI_DEVICE_1
#define USE_SPI_DEVICE_2
//#define USE_SPI_DEVICE_3

/*
#define USE_I2C
#define I2C_DEVICE_INT (I2CDEV_1)
#define I2C_DEVICE_EXT (I2CDEV_2)
*/


//#define SENSORS_SET (SENSOR_ACC|SENSOR_BARO)

#define SENSORS_SET (SENSOR_ACC)


#define USE_ADC
#define BOARD_HAS_VOLTAGE_DIVIDER


#define ADC_INSTANCE                ADC2
#define ADC_DMA_CHANNEL             DMA2_Channel1
#define ADC_AHB_PERIPHERAL          RCC_AHBPeriph_DMA2

#define VBAT_ADC_GPIO               GPIOC
#define VBAT_ADC_GPIO_PIN           GPIO_Pin_0
#define VBAT_ADC_CHANNEL            ADC_Channel_1

#define CURRENT_METER_ADC_GPIO      GPIOA
#define CURRENT_METER_ADC_GPIO_PIN  GPIO_Pin_5
#define CURRENT_METER_ADC_CHANNEL   ADC_Channel_2

#define RSSI_ADC_GPIO               GPIOB
#define RSSI_ADC_GPIO_PIN           GPIO_Pin_2
#define RSSI_ADC_CHANNEL            ADC_Channel_12


//#define LED_STRIP
//#define LED_STRIP_TIMER TIM5

#define GPS
#define BLACKBOX
#define TELEMETRY
#define SERIAL_RX
#define AUTOTUNE
#define USE_SERVOS
#define GTUNE
#define USE_CLI

#define USE_SERIAL_1WIRE

// STM32F103CBT6-LQFP48 Pin30 (PA9) TX - PC3 connects to onboard CP2102 RX
#define S1W_TX_GPIO         GPIOA
#define S1W_TX_PIN          GPIO_Pin_9
// STM32F103CBT6-LQFP48 Pin31 (PA10) RX - PC1 to onboard CP2102 TX
#define S1W_RX_GPIO         GPIOA
#define S1W_RX_PIN          GPIO_Pin_10

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DISPALY_SPI1_CS_Pin GPIO_PIN_3
#define DISPALY_SPI1_CS_GPIO_Port GPIOA
#define DISPALY_SPI1_RST_Pin GPIO_PIN_4
#define DISPALY_SPI1_RST_GPIO_Port GPIOA
#define DISPLAY_SPI1_SCK_Pin GPIO_PIN_5
#define DISPLAY_SPI1_SCK_GPIO_Port GPIOA
#define DISPALY_SPI1_DC_Pin GPIO_PIN_6
#define DISPALY_SPI1_DC_GPIO_Port GPIOA
#define DISPLAY_SPI1_MOSI_Pin GPIO_PIN_7
#define DISPLAY_SPI1_MOSI_GPIO_Port GPIOA
#define TP_TINT_Pin GPIO_PIN_0
#define TP_TINT_GPIO_Port GPIOB
#define EM7028_INT_Pin GPIO_PIN_1
#define EM7028_INT_GPIO_Port GPIOB
#define FLASH_SPI2_SCK_Pin GPIO_PIN_10
#define FLASH_SPI2_SCK_GPIO_Port GPIOB
#define FLASH_SPI2_CS_Pin GPIO_PIN_13
#define FLASH_SPI2_CS_GPIO_Port GPIOB
#define FLASH_SPI2_MISO_Pin GPIO_PIN_14
#define FLASH_SPI2_MISO_GPIO_Port GPIOB
#define FLASH_SPI2_MOSI_Pin GPIO_PIN_15
#define FLASH_SPI2_MOSI_GPIO_Port GPIOB
#define SENSOR_I2C3_SCL_Pin GPIO_PIN_8
#define SENSOR_I2C3_SCL_GPIO_Port GPIOA
#define WT_BUSY_Pin GPIO_PIN_12
#define WT_BUSY_GPIO_Port GPIOA
#define SENSOR_I2C3_SDA_Pin GPIO_PIN_4
#define SENSOR_I2C3_SDA_GPIO_Port GPIOB
#define MPU6050_INT_Pin GPIO_PIN_5
#define MPU6050_INT_GPIO_Port GPIOB
#define MPU6050_INT_EXTI_IRQn EXTI9_5_IRQn
#define TP_I2C1_SCL_Pin GPIO_PIN_6
#define TP_I2C1_SCL_GPIO_Port GPIOB
#define TP_I2C1_SDA_Pin GPIO_PIN_7
#define TP_I2C1_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

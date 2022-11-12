/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32l0xx_hal.h"

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
#define BUT_OK_Pin GPIO_PIN_0
#define BUT_OK_GPIO_Port GPIOA
#define BUT_BACK_Pin GPIO_PIN_1
#define BUT_BACK_GPIO_Port GPIOA
#define DISP_CS_Pin GPIO_PIN_4
#define DISP_CS_GPIO_Port GPIOA
#define DISP_SCK_Pin GPIO_PIN_5
#define DISP_SCK_GPIO_Port GPIOA
#define DISP_RES_Pin GPIO_PIN_6
#define DISP_RES_GPIO_Port GPIOA
#define DISP_MOSI_Pin GPIO_PIN_7
#define DISP_MOSI_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_0
#define LED_GPIO_Port GPIOB
#define IR_LED_Pin GPIO_PIN_1
#define IR_LED_GPIO_Port GPIOB
#define DISP_DC_Pin GPIO_PIN_8
#define DISP_DC_GPIO_Port GPIOA
#define BUT_UP_Pin GPIO_PIN_9
#define BUT_UP_GPIO_Port GPIOA
#define BUT_DOWN_Pin GPIO_PIN_10
#define BUT_DOWN_GPIO_Port GPIOA
#define IBT_EMU_Pin GPIO_PIN_11
#define IBT_EMU_GPIO_Port GPIOA
#define IBT_RW_Pin GPIO_PIN_12
#define IBT_RW_GPIO_Port GPIOA
#define IR_REC_Pin GPIO_PIN_15
#define IR_REC_GPIO_Port GPIOA
#define BAT_ALRT_Pin GPIO_PIN_3
#define BAT_ALRT_GPIO_Port GPIOB
#define POW_PERIPH_Pin GPIO_PIN_4
#define POW_PERIPH_GPIO_Port GPIOB
#define BAT_SCL_Pin GPIO_PIN_6
#define BAT_SCL_GPIO_Port GPIOB
#define BAT_SDA_Pin GPIO_PIN_7
#define BAT_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

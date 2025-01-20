/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "esp.h"
#include "rcp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /*__GNUC__*/

#define DEBUGGING 0
#define MOTOR_DELAY 3
#define MOTOR_MOVE 5
#define GRIP_MOVE 15

#define ARR_CNT 5
#define CMD_SIZE 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

//UART_HandleTypeDef huart2;
//UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
uint8_t rx2char;
extern cb_data_t cb_data;
extern volatile unsigned char rx2Flag;
extern volatile char rx2Data[50];

uint16_t servo1_init_pwm, servo2_init_pwm, servo3_init_pwm, servo4_init_pwm, gripper_init_pwm;

typedef struct _pwm_data
{
	long servo1_pwm;
	long servo2_pwm;
	long servo3_pwm;
} pwm_data;

pwm_data servo_data;

float grip_pos[] = {140, 0, 165};
float red_pos[] = {0, 180, 150};
float green_pos[] = {-127, 127 , 150};
float blue_pos[] = {-180, 0, 150};

int grip_flag;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
//static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
//static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
void mathmatics(float pos[], pwm_data* servo_data);
void servo_Init();
void move_direction(pwm_data* servo_data);

char strBuff[MAX_ESP_COMMAND_LEN];
void MX_GPIO_LED_ON(int flag);
void MX_GPIO_LED_OFF(int flag);
void esp_event(char *);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	int ret = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  setvbuf(stdin, NULL, _IONBF, 0);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
//  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
//  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  ret |= drv_uart_init();
  ret |= drv_esp_init();
  if(ret != 0) Error_Handler();
  printf("Start main() \r\n");
  AiotClient_Init();

  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);

  printf("servo initializing...\r\n");
  servo_Init();
  printf("servo is ready\r\n");

  printf("start main() - mathmatics\n\r");

//  pwm_data servo_data;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	   /*
	   **servo_motor_range**
	   standard : 84MHz, prescaler 84, count period : 2000 -> 500 ~ 2500
	   - gripper : 1300 ~ 2300, init : 1300
	   - servo 1 : 500 ~ 2500, init : 1510
	   - servo 2 : 500 ~ 2500, init : 1510
	   - servo 3 : 900 ~ 2500, init : 2490
	   - servo 4 : 1500 fix (roll rotate servo, we do not use it), init : 1480
	   TIM3 CH1 = joint 1 : PA6
	   TIM3 CH2 = joint 2 : PA7
	   TIM3 CH3 = joint 3 : PB0
	   TIM3 CH4 = joint 4 : PB1 **NOT USE** always 0 degree
	   TIM4 CH1 = gripper : PB6
	   */

	  	/*
	  	float px = 0, py = 0, pz = 0;

		printf("Enter the end effect coordinate\n\r");
		printf("px : \r\n");
		scanf("%f", &px);
		printf("py : \r\n");
		scanf("%f", &py);
		printf("pz : \r\n");
		scanf("%f", &pz);

		float enter_pos[] = {px, py, pz};
		*/

//		if(strstr((char *)cb_data.buf,"+IPD") && cb_data.buf[cb_data.length-1] == '\n')
//		{
//			//?��?��?���??  \r\n+IPD,15:[KSH_LIN]HELLO\n
//			// copy received string
//			strcpy(strBuff,strchr((char *)cb_data.buf,'['));
//			memset(cb_data.buf,0x0,sizeof(cb_data.buf));
//			cb_data.length = 0;
//			esp_event(strBuff);
//		}
		if(cb_data.buf[cb_data.length-1] == '\n')
		{
			// ex \r\n+IPD,15:[KSH_LIN]HELLO\n
			// copy received string
			strcpy(strBuff,(char *)cb_data.buf);
			memset(cb_data.buf,0x0,sizeof(cb_data.buf));
			cb_data.length = 0;
			esp_event(strBuff);
		}
		if(rx2Flag)
		{
			printf("recv2 : %s\r\n",rx2Data);
			rx2Flag =0;
		}

		/*
	  	float grip_pos[] = {140, 0, 165};
	  	float red_pos[] = {0, 180, 150};
	  	float green_pos[] = {-127, 127 , 150};
	  	float blue_pos[] = {-180, 0, 150};
		*/
	  	int grip_btn = HAL_GPIO_ReadPin(GRIP_BTN_GPIO_Port, GRIP_BTN_Pin);
	  	int red_btn = HAL_GPIO_ReadPin(RED_BTN_GPIO_Port, RED_BTN_Pin);
	  	int green_btn = HAL_GPIO_ReadPin(GREEN_BTN_GPIO_Port, GREEN_BTN_Pin);
	  	int blue_btn = HAL_GPIO_ReadPin(BLUE_BTN_GPIO_Port, BLUE_BTN_Pin);

	  	if (grip_btn == 1)
	  	{
	  		grip_flag = 1;
	  		mathmatics(grip_pos, &servo_data);
			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
	  		move_direction(&servo_data);
	  		grip_flag = 0;
	  	}
	  	if (red_btn == 1)
		{
			mathmatics(red_pos, &servo_data);
			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
			move_direction(&servo_data);
		}
	  	if (green_btn == 1)
		{
			mathmatics(green_pos, &servo_data);
			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
			move_direction(&servo_data);
		}
	  	if (blue_btn == 1)
		{
			mathmatics(blue_pos, &servo_data);
			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
			move_direction(&servo_data);
		}


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 20000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 84-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 20000-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
#if 0
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}
#endif
/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GRIP_BTN_Pin RED_BTN_Pin GREEN_BTN_Pin BLUE_BTN_Pin */
  GPIO_InitStruct.Pin = GRIP_BTN_Pin|RED_BTN_Pin|GREEN_BTN_Pin|BLUE_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void servo_Init()
{
	  /*
	   **servo_motor_range**
	   standard : 84MHz, prescaler 84, count period : 2000 -> 500 ~ 2500
	   - gripper : 1300 ~ 2300, init : 1300
	   - servo 1 : 500 ~ 2500, init : 1510
	   - servo 2 : 500 ~ 2500, init : 1510
	   - servo 3 : 900 ~ 2500, init : 2490
	   - servo 4 : 1500 fix (roll rotate servo, we do not use it), init : 1480
	   TIM3 CH1 = joint 1 : PA6
	   TIM3 CH2 = joint 2 : PA7
	   TIM3 CH3 = joint 3 : PB0
	   TIM3 CH4 = joint 4 : PB1 **NOT USE** always 0 degree
	   TIM4 CH1 = gripper : PB6
	   */

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 1510);
	HAL_Delay(1000);
	servo1_init_pwm = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1);

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 1510);
	HAL_Delay(1000);
	servo2_init_pwm = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2);

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 2490);
	HAL_Delay(1000);
	servo3_init_pwm = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_3);

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 1480);
	HAL_Delay(1000);
	servo4_init_pwm = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_4);

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1300);
	HAL_Delay(1000);
	gripper_init_pwm = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_1);
}

void mathmatics(float pos[], pwm_data* servo_data)
{
	float px = pos[0], py = pos[1], pz = pos[2];	// end point coordinate
	float theta1 = 0, theta2 = 0, theta3 = 0;	// joint degree variable
	float R = 0, Q = 0, P = 0, K = 0;
	float gamma = 0, pi = 0, phi = 0, beta = 0;
	int d1 = 83, a2 = 67, a3 = 60, d4 = 76, a4 = 15;	// link length (mm)

	theta1 = atan2(py, px) * 180 / M_PI;
	long servo1_pwm = (theta1/180*2000) + 500;
	(*servo_data).servo1_pwm = servo1_pwm;

	R = sqrt(pow(px, 2) + pow(py, 2));
	Q = sqrt(pow(R, 2) + pow(pz, 2));
	P = sqrt(pow(R, 2) + pow((d1 - pz), 2));
	K = sqrt(pow((a3 + d4), 2) + pow(a4, 2));

	float gamma_y = pow(P, 2) + pow(d1, 2) - pow(Q, 2);
	float gamma_x = 2 * P * d1;
	gamma = acos(gamma_y / gamma_x) * 180 / M_PI;

	float pi_y = pow(P, 2) + pow(a2, 2) - pow(K, 2);
	float pi_x = 2 * P * a2;
	pi = acos(pi_y / pi_x) * 180 / M_PI;

	theta2 = 180 - (pi - (90 - gamma));
	long servo2_pwm = theta2/180.0*2000.0 + 500;
	(*servo_data).servo2_pwm = servo2_pwm;

	float phi_y = pow(a2, 2) + pow(K, 2) - pow(P, 2);
	float phi_x = 2 * a2 * K;
	phi = acos(phi_y / phi_x) * 180 / M_PI;
	beta = atan2(a4, a3 + d4) * 180 / M_PI;

	theta3 = 180 - (phi + beta);
	long servo3_pwm = 2500 - theta3/180.0*2000.0;
	(*servo_data).servo3_pwm = servo3_pwm;

#if DEBUGGING
	printf("R : %f\r\n", R);
	printf("Q : %f\r\n", Q);
	printf("P : %f\r\n", P);
	printf("K : %f\r\n", K);
	printf("gamma_y : %f\r\n", gamma_y);
	printf("gamma_x : %f\r\n", gamma_x);
	printf("gamma : %f\r\n", gamma);
	printf("pi_y : %f\r\n", pi_y);
	printf("pi_x : %f\r\n", pi_x);
	printf("pi : %f\r\n", pi);
	printf("phi_y : %f\r\n", phi_y);
	printf("phi_x : %f\r\n", pi_x);
	printf("phi : %f\r\n", phi);
	printf("beta : %f\r\n", beta);
	printf("theta1 : %.2f, theta2 : %.2f, theta3 : %.2f\n\r", theta1, theta2, theta3);
	printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\n\r", servo1_pwm, servo2_pwm, servo3_pwm);
#endif

}

void move_direction(pwm_data* servo_data)
{
	long m_servo1_pwm = servo_data->servo1_pwm;
	long m_servo2_pwm = servo_data->servo2_pwm;
	long m_servo3_pwm = servo_data->servo3_pwm;
	long m_grip_pwm = 2400;

	long servo1_current_pwm = 0, servo2_current_pwm = 0, servo3_current_pwm = 0;

	//move to set position
	//servo1 move
	if(m_servo1_pwm >= servo1_init_pwm)
	{
		for(servo1_current_pwm = servo1_init_pwm; servo1_current_pwm < m_servo1_pwm; servo1_current_pwm+=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, servo1_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}
	else if(m_servo1_pwm < servo1_init_pwm)
	{
		for(servo1_current_pwm = servo1_init_pwm; servo1_current_pwm > m_servo1_pwm; servo1_current_pwm-=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, servo1_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}

	//servo2 move
	if(m_servo2_pwm >= servo2_init_pwm)
	{
		for(servo2_current_pwm = servo2_init_pwm; servo2_current_pwm < m_servo2_pwm; servo2_current_pwm+=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, servo2_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}
	else if(m_servo2_pwm < servo2_init_pwm)
	{
		for(servo2_current_pwm = servo2_init_pwm; servo2_current_pwm > m_servo2_pwm; servo2_current_pwm-=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, servo2_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}

	//servo3 move
	if(m_servo3_pwm >= servo3_init_pwm)
	{
		for(servo3_current_pwm = servo3_init_pwm; servo3_current_pwm < m_servo3_pwm; servo3_current_pwm+=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, servo3_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}
	else if(m_servo3_pwm < servo3_init_pwm)
	{
		for(servo3_current_pwm = servo3_init_pwm; servo3_current_pwm > m_servo3_pwm; servo3_current_pwm-=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, servo3_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}

	//gripper
	if(grip_flag == 1)
	{
		for(long grip_cur = gripper_init_pwm; grip_cur < m_grip_pwm; grip_cur+=GRIP_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, grip_cur);
			HAL_Delay(MOTOR_DELAY);
		}
	}
	if(grip_flag == 0)
	{
		for(long grip_cur = m_grip_pwm; grip_cur > gripper_init_pwm; grip_cur-=GRIP_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, grip_cur);
			HAL_Delay(MOTOR_DELAY);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

	//move back to initial position
	//servo2 move back
	if(servo2_init_pwm >= m_servo2_pwm)
	{
		for(servo2_current_pwm = m_servo2_pwm; servo2_current_pwm < servo2_init_pwm; servo2_current_pwm+=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, servo2_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}
	else if(servo2_init_pwm < m_servo2_pwm)
	{
		for(servo2_current_pwm = m_servo2_pwm; servo2_current_pwm > servo2_init_pwm; servo2_current_pwm-=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, servo2_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}

	//servo3 move back
	if(servo3_init_pwm >= m_servo3_pwm)
	{
		for(servo3_current_pwm = m_servo3_pwm; servo3_current_pwm < servo3_init_pwm; servo3_current_pwm+=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, servo3_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}
	else if(servo3_init_pwm < m_servo3_pwm)
	{
		for(servo3_current_pwm = m_servo3_pwm; servo3_current_pwm > servo3_init_pwm; servo3_current_pwm-=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, servo3_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}

	//servo1 move back
	if(servo1_init_pwm >= m_servo1_pwm)
	{
		for(servo1_current_pwm = m_servo1_pwm; servo1_current_pwm < servo1_init_pwm; servo1_current_pwm+=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, servo1_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}
	else if(servo1_init_pwm < m_servo1_pwm)
	{
		for(servo1_current_pwm = m_servo1_pwm; servo1_current_pwm > servo1_init_pwm; servo1_current_pwm-=MOTOR_MOVE)
		{
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, servo1_current_pwm);
			HAL_Delay(MOTOR_DELAY);
		}

	}

	/*
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, m_servo1_pwm);
	HAL_Delay(1000);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, m_servo2_pwm);
	HAL_Delay(1000);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, m_servo3_pwm);
	HAL_Delay(1000);

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, servo2_init_pwm);
	HAL_Delay(1000);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, servo3_init_pwm);
	HAL_Delay(1000);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, servo1_init_pwm);
	HAL_Delay(1000);
	*/
}

#if 0
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

GETCHAR_PROTOTYPE
{
  uint8_t ch = 0;

  /* Clear the Overrun flag just before receiving the first character */
  __HAL_UART_CLEAR_OREFLAG(&huart2);

  /* Wait for reception of a character on the USART RX line and echo this
   * character on console */
  HAL_UART_Receive(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
#endif

void MX_GPIO_LED_ON(int pin)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, pin, GPIO_PIN_SET);
}
void MX_GPIO_LED_OFF(int pin)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, pin, GPIO_PIN_RESET);
}

/* 의근님 코드 START */

//void esp_event(char * recvBuf)
//{
//  int i=0;
//  char * pToken;
//  char * pArray[ARR_CNT]={0};
//  char sendBuf[MAX_UART_COMMAND_LEN]={0};
//  RCT_Instance rct_data;
//
//  // Arrays of pArray is used to receive server information and data.
//  // pArray[0]: server information
//  // pArray[1]: Flag about state of conveyer
//  // pArray[2]: color information
//
//  strBuff[strlen(recvBuf)-1] = '\0';	//'\n' cut
//  printf("\r\nDebug recv : %s\r\n",recvBuf);
//
//  pToken = strtok(recvBuf,"[@]");
//  while(pToken != NULL)
//  {
//    pArray[i] = pToken;
//    if(++i >= ARR_CNT)
//      break;
//    pToken = strtok(NULL,"[@]");
//  }
//
//  if(!strcmp(pArray[1],"LED"))
//  {
//  	if(!strcmp(pArray[2],"ON"))
//  	{
//  		MX_GPIO_LED_ON(LD2_Pin);
//
//  	}
//		else if(!strcmp(pArray[2],"OFF"))
//		{
//				MX_GPIO_LED_OFF(LD2_Pin);
//		}
//		sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
//  }
//
//  else if() {
//
//  }
//
//  // Transmit message about gripper is gripping
//  else if(!strcmp(pArray[1],"1"))
//  {
//	  	sprintf(sendBuf, "[%s]OBJECT_GRIPPING\n",pArray[0]);
//	  	esp_send_data(sendBuf);
//
//	  	// Flag of grip_flag is used for function of move_direction
//	  	// Flag of grip_flag control gripper
//
//	  	// Gripper is gripping
//		grip_flag = 1;
//		mathmatics(grip_pos, &servo_data);
//		printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
//		move_direction(&servo_data);
//
//		// Gripper is hold
//		grip_flag = 0;
//
//		// Color: Red
//		// Transmit message about gripping is done.
//		if(!strcmp(pArray[2], "1"))
//		{
//			mathmatics(red_pos, &servo_data);
//			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
//			move_direction(&servo_data);
//			sprintf(sendBuf, "[%s]DONE\n",pArray[0]);
//			esp_send_data(sendBuf);
//		}
//		// Color: Green
//		// Transmit message about gripping is done.
//		else if(!strcmp(pArray[2], "2"))
//		{
//			mathmatics(green_pos, &servo_data);
//			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
//			move_direction(&servo_data);
//			sprintf(sendBuf, "[%s]DONE\n",pArray[0]);
//			esp_send_data(sendBuf);
//		}
//		// Color: Blue
//		// Transmit message about gripping is done.
//		else if(!strcmp(pArray[2], "3"))
//		{
//			mathmatics(blue_pos, &servo_data);
//			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
//			move_direction(&servo_data);
//			sprintf(sendBuf, "[%s]DONE\n",pArray[0]);
//			esp_send_data(sendBuf);
//		}
//  }
//
//  else if(!strncmp(pArray[1]," New conn",8))
//  {
////	   printf("Debug : %s, %s\r\n",pArray[0],pArray[1]);
//     return;
//  }
//  else if(!strncmp(pArray[1]," Already log",8))
//  {
//// 	    printf("Debug : %s, %s\r\n",pArray[0],pArray[1]);
//			esp_client_conn();
//      return;
//  }
//  else
//      return;
//
////  esp_send_data(sendBuf);
////  printf("Debug send : %s\r\n",sendBuf);
//  printf("NOW STATE : %s\r\n",sendBuf);
//}

/* 의근님 코드 END */

/* 정우 코드 START */

void esp_event(char *recvBuf)
{
    //int i = 0;
    //char *pToken;
    char *pArray[ARR_CNT] = {0};  // pArray에 값을 저장할 배열
    char sendBuf[MAX_UART_COMMAND_LEN] = {0};

    RCT_Instance rct_data;

    // 예시로 recvBuf를 수신했다고 가정
    //printf("\r\nDebug recv : %s\r\n", recvBuf);
    recvBuf[strlen(recvBuf) - 1] = '\0';  // '\n'을 제거

    // recvBuf에서 구분자 '[@]'를 기준으로 문자열 분리
//    pToken = strtok(recvBuf, "[@]");
//    while (pToken != NULL)
//    {
//        pArray[i] = pToken;
//        if (++i >= ARR_CNT)
//            break;
//        pToken = strtok(NULL, "[@]");
//    }

   strcpy(pArray[0], LOGID);

    // 수신된 데이터를 기반으로 필요한 값 설정
    // RCT_Instance 구조체로 수신 데이터를 파싱하여 flag와 color 값을 pArray에 저장
    deserialize_rct_data((uint8_t *)recvBuf, &rct_data);
    // debug
    //test_rcp(&rct_data);
    printf("%d %d\n", rct_data.rc_data.detect_flag, rct_data.rc_data.color);

	// pdu.rcp_header.flag을 pArray[1]에 저장
	//pArray[1] = malloc(sizeof(char) * 5); // 충분한 메모리 할당
	//snprintf(pArray[1], 5, "%d", rct_data.rc_data.detect_flag); // flag 값을 pArray[1]에 문자열로 저장

	// pdu.rc_data.color를 pArray[2]에 저장
	//pArray[2] = malloc(sizeof(char) * 5); // 충분한 메모리 할당
	//snprintf(pArray[2], 5, "%d", rct_data.rc_data.color); // color 값을 pArray[2]에 문자열로 저장

    // 그 이후의 처리는 기존 코드와 동일하게 진행
//    if (!strcmp(pArray[1], "LED"))
//    {
//        if (!strcmp(pArray[2], "ON"))
//        {
//            MX_GPIO_LED_ON(LD2_Pin);
//        }
//        else if (!strcmp(pArray[2], "OFF"))
//        {
//            MX_GPIO_LED_OFF(LD2_Pin);
//        }
//        sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]);
//    }

	//
	//if (verify_checksum(&rct_data)) {
		//rct_data.rc_data.detect_flag;
		//if ((int)pArray[1] == DETECT)
		if (rct_data.rc_data.detect_flag == DETECT)
		{
			//sprintf(sendBuf, "[%s]OBJECT_GRIPPING\n", pArray[0]);
			//esp_send_data(sendBuf);
			grip_flag = 1;
			mathmatics(grip_pos, &servo_data);
			printf("pArray[1]: %d\n", (int)pArray[1]);
			printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
			move_direction(&servo_data);
			grip_flag = 0;

			//if ((int)pArray[2] == RED)
			if (rct_data.rc_data.color == RED)
			{
				mathmatics(red_pos, &servo_data);
				printf("pArray[2]: %d\n", (int)pArray[2]);
				printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
				move_direction(&servo_data);

				//sprintf(sendBuf, "[%s]RED_ACT_DONE_ARM_READY\n", pArray[0]);
				//sprintf(sendBuf, "READY");
				//esp_send_data(sendBuf);
				//test_rcp(&rct_data);
				rct_send_confirm_ready_message(sendBuf);
			}
			//else if ((int)pArray[2] == GREEN)
			else if (rct_data.rc_data.color == GREEN)
			{
				mathmatics(green_pos, &servo_data);
				printf("pArray[2]: %d\n", (int)pArray[2]);
				printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
				move_direction(&servo_data);
				//sprintf(sendBuf, "[%s]GREEN_ACT_DONE_ARM_READY\n", pArray[0]);
				//sprintf(sendBuf, "READY", pArray[0]);
				//esp_send_data(sendBuf);
				rct_send_confirm_ready_message(sendBuf);
			}
			//else if ((int)pArray[2] == BLUE)
			else if (rct_data.rc_data.color == BLUE)
			{
				mathmatics(blue_pos, &servo_data);
				printf("pArray[2]: %d\n", (int)pArray[2]);
				printf("servo1_pwm : %ld, servo2_pwm : %ld, servo3_pwm : %ld\r\n", servo_data.servo1_pwm, servo_data.servo2_pwm, servo_data.servo3_pwm);
				move_direction(&servo_data);
				//sprintf(sendBuf, "[%s]BLUE_ACT_DONE_ARM_READY\n", pArray[0]);
				//sprintf(sendBuf, "READY", pArray[0]);
				//esp_send_data(sendBuf);
				rct_send_confirm_ready_message(sendBuf);
			}
		}
		else if (!strncmp(pArray[1], " New conn", 8))
		{
			return;
		}
		else if (!strncmp(pArray[1], " Already log", 8))
		{
			esp_client_conn();
			return;
		}
		else
			return;
	//}

    //printf("NOW STATE : %s\r\n", sendBuf);
}

/* 정우 코드 END */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

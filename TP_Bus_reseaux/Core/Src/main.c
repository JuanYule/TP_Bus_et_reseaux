/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "BMP280.h"	//Importe librarie pour contrôler le capteur
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIZE_REQUETE 10
#define ID_MESSAGE_CAN 0x61 //two valid values 0x60 (manual mode) 0x61 (angle mode)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim13;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;

/* USER CODE BEGIN PV */
uint8_t data[6];
uint8_t REG_ID = 0xD0;

//Variables pour calibration
uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;
uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

//variables reception raspberry
uint8_t Rx_rasp_to_nucleo = 1;
char requete[SIZE_REQUETE] = {'\0'};
int flag = 0;

//Variables reception Rx UART1
uint8_t Rx_byte;
char Rx_data[SIZE_REQUETE] = {'\0'};
int int_temp, dec_temp;
uint8_t Rx_indx = 0;
BMP280_U32_t send_temp, send_press; //valeurs a envoyer quand la raspberry demande un request

char send_t_1[2];
char send_t_2[2];

char send_p[2];

char t_display [10];
char p_display [10];

////Structure Header bus CAN
CAN_TxHeaderTypeDef pHeader;
uint8_t aData[2]= {0x01,0x00};
uint32_t pTxMailbox;
int incr = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM13_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void configBMP280(){
	uint8_t pData[2];
	pData[0] = 0xF4; // @ du ctrl_meas
	pData[1] = 0x57; //config mode normal, oversampling pressure x16 et temp x2
	HAL_I2C_Master_Transmit(&hi2c1, addr_BMP280, pData, 2, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, addr_BMP280, &data[0], 6, HAL_MAX_DELAY);
	printf("la configuration est faite et la nouvelle valeur du registre est : %x\r\n", data[0]);

}

void id_BMP280(){
	HAL_I2C_Master_Transmit(&hi2c1, addr_BMP280, &REG_ID, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, addr_BMP280, &data[0], 6, HAL_MAX_DELAY);
	if(data[0]==0x58){
		printf("Le composant est le BMP280 et son id est : %x\r\n", data[0]);
	}
	else{
		printf("Le composant n'est pas reconnu");
	}
}

void etalonnageBMP280(){
	// Lecture des «trimming parameters»
	uint8_t calibration[26]; //tableau des variables
	uint8_t REG_trimming_parameter = 0x88; //premier address
	//int i = 0;  //variable d'itération
	//	for (i = 0; i < sizeof(REG_trimming_parameter); ++i) {
	//		REG_trimming_parameter[i] = 0x88 + i;
	//		printf("register: %x \r\n",REG_trimming_parameter[i]);
	//	}
	HAL_I2C_Master_Transmit(&hi2c1, addr_BMP280, &REG_trimming_parameter, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, addr_BMP280, calibration, 26, HAL_MAX_DELAY);

	//	for ( i = 0; i < 25; ++i) {
	//		printf("Valeur premiere addr: %x\r\n",calibration[i]);
	//	}
	dig_T1 = (uint16_t)calibration[1] << 8 | calibration[0];
	dig_T2 = (int16_t)calibration[3] << 8 | calibration[2];
	dig_T3 = (int16_t)calibration[5] << 8 | calibration[4];
	dig_P1 = (uint16_t)calibration[7] << 8 | calibration[6];
	dig_P2 = (int16_t)calibration[9] << 8 | calibration[8];
	dig_P3 = (int16_t)calibration[11] << 8 | calibration[10];
	dig_P4 = (int16_t)calibration[13] << 8 | calibration[12];
	dig_P5 = (int16_t)calibration[15] << 8 | calibration[14];
	dig_P6 = (int16_t)calibration[17] << 8 | calibration[16];
	dig_P7 = (int16_t)calibration[19] << 8 | calibration[18];
	dig_P8 = (int16_t)calibration[21] << 8 | calibration[20];
	dig_P9 = (int16_t)calibration[23] << 8 | calibration[22];
	//dig_T1 = (uint16_t)calibration[1] << 8 | calibration[0];
}

//==============Compensation formula in 32 bit fixed point
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BMP280_S32_t t_fine;
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
	BMP280_S32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) *
			((BMP280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)
{
	BMP280_S32_t var1, var2;
	BMP280_U32_t p;
	var1 = (((BMP280_S32_t)t_fine)>>1) - (BMP280_S32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BMP280_S32_t)dig_P6);
	var2 = var2 + ((var1*((BMP280_S32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((BMP280_S32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BMP280_S32_t)dig_P2) * var1)>>1))>>18;
	var1 =((((32768+var1))*((BMP280_S32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((BMP280_U32_t)(((BMP280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((BMP280_U32_t)var1);
	}
	else
	{
		p = (p / (BMP280_U32_t)var1) * 2;
	}
	var1 = (((BMP280_S32_t)dig_P9) * ((BMP280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((BMP280_S32_t)(p>>2)) * ((BMP280_S32_t)dig_P8))>>13;
	p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}

BMP280_S32_t read_temp()
{
	uint8_t addr_temp_press = 0xF7;

	uint8_t data_temp_press[6];
	uint32_t temperature;
	BMP280_S32_t temp_compensation;

	HAL_I2C_Master_Transmit(&hi2c1, addr_BMP280, &addr_temp_press, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, addr_BMP280, data_temp_press, 6, HAL_MAX_DELAY);
	
	temperature = (data_temp_press[3]<<12)|(data_temp_press[4]<<4)|(data_temp_press[5]>>4);


	temp_compensation = bmp280_compensate_T_int32(temperature);
	//printf("T=+%2d.%2d_C \r\n",(int)(temp_compensation/100),temp_compensation%100);
	return temp_compensation;
}
BMP280_U32_t read_press()
{
	uint8_t addr_temp_press = 0xF7;
	uint8_t data_temp_press[6];
	uint32_t pressure;
	BMP280_U32_t press_compensation;


	HAL_I2C_Master_Transmit(&hi2c1, addr_BMP280, &addr_temp_press, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, addr_BMP280, data_temp_press, 6, HAL_MAX_DELAY);

	pressure = (data_temp_press[0]<<12)|(data_temp_press[1]<<4)|(data_temp_press[2]>>4);
	press_compensation=bmp280_compensate_P_int32(pressure);
	printf("P=%10dPa \r\n", (int)(press_compensation/100));
	return press_compensation;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART3) { // Current UART
		Rx_data[Rx_indx++] = Rx_byte;    // Add data to Rx_Buffer
	}

	if (strcmp(Rx_data,"GET_T") == 0)
	{
		//send_temp = read_temp();
		int_temp = send_temp/100;
		dec_temp = send_temp % 100;
		sprintf(send_t_1, "%d", int_temp);
		sprintf(send_t_2, "%d", dec_temp);
		strcat(t_display, "T=+");
		strcat(p_display, ".");
		strcat(t_display, strcat(send_t_1,strcat(p_display,strcat(send_t_2,"_C"))));
		printf("T=+%d.%d_C \r\n",int_temp, dec_temp);
		HAL_UART_Transmit(&huart3, (uint8_t*)&t_display, strlen(t_display), HAL_MAX_DELAY);
		memset(Rx_data, 0, SIZE_REQUETE);
		Rx_indx = 0;
	}
	else if (strcmp(Rx_data,"GET_P") == 0)
	{
		HAL_UART_Transmit(&huart3, (uint8_t*)&Rx_data, strlen(Rx_data), HAL_MAX_DELAY);
		memset(Rx_data, 0, SIZE_REQUETE);
		Rx_indx = 0;
	}
	else if (strcmp(Rx_data,"GET_K") == 0)
	{
		HAL_UART_Transmit(&huart3, (uint8_t*)&Rx_data, strlen(Rx_data), HAL_MAX_DELAY);
		memset(Rx_data, 0, SIZE_REQUETE);
		Rx_indx = 0;
	}
	else if (strcmp(Rx_data,"GET_A") == 0)
	{
		HAL_UART_Transmit(&huart3, (uint8_t*)&Rx_data, strlen(Rx_data), HAL_MAX_DELAY);
		memset(Rx_data, 0, SIZE_REQUETE);
		Rx_indx = 0;
	}
	else if (strcmp(Rx_data,"SET_K") == 0)
	{
		HAL_UART_Transmit(&huart3, (uint8_t*)&Rx_data, strlen(Rx_data), HAL_MAX_DELAY);
		memset(Rx_data, 0, SIZE_REQUETE);
		Rx_indx = 0;
	}
	HAL_UART_Receive_DMA(&huart3, &Rx_byte, 1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	if (htim->Instance == TIM13)
	{

		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		//		if(incr==1){
		send_temp = read_temp();
		printf("T=+%2d.%2d_C \r\n",(int)(send_temp/100),send_temp%100);
		int_temp = (send_temp/100)*10;
		aData[1]=0x01;
		aData[0]=int_temp;
		incr =0;
		//		}
		//		else{
		////			aData[1] = 0x00;
		////			aData[0] = 0x5A;
		//		}
		HAL_CAN_AddTxMessage(&hcan1, &pHeader, aData, &pTxMailbox);
		incr = 1;
	}
}

void config_CAN(){
	//CAN configuration
	aData[0] = 0x00;
	aData[1] = 0x00;
	pHeader.StdId = ID_MESSAGE_CAN;
	//pHeader.ExtId =0x000;
	pHeader.IDE = CAN_ID_STD;
	pHeader.RTR = CAN_RTR_DATA;
	pHeader.DLC = 2;
	pHeader.TransmitGlobalTime = DISABLE;
}

void config_CAN_set0(){
	//CAN configuration
	pHeader.StdId = 0x62;
	//pHeader.ExtId =0x000;
	pHeader.IDE = CAN_ID_STD;
	pHeader.RTR = CAN_RTR_DATA;
	pHeader.DLC = 2;
	pHeader.TransmitGlobalTime = DISABLE;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	MX_CAN1_Init();
	MX_I2C1_Init();
	MX_USART3_UART_Init();
	MX_TIM13_Init();
	/* USER CODE BEGIN 2 */

	//Configuration du capteur BMP280
	configBMP280();

	//Calibration || etalonnage
	printf("Etalonnage\r\n");
	etalonnageBMP280();

	read_temp();

	// On active UART avec le DMA, on va recevoir un seul byte
	HAL_UART_Receive_DMA(&huart3, &Rx_byte, 1); //On

	//init Timer 13
	HAL_TIM_Base_Start_IT(&htim13);
	HAL_CAN_Start(&hcan1); //init bus CAN

	//config_CAN_set0();

	config_CAN(); //Fonction pour faire la conf du bus CAN
	HAL_CAN_AddTxMessage(&hcan1, &pHeader, aData, &pTxMailbox);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		//		if (HAL_UART_Receive(&huart3, &Rx_rasp_to_nucleo, 1, 1000)==HAL_OK) {
		//
		//			if (Rx_rasp_to_nucleo==0x0D) {
		////				printf("Data = %s \r\n", requete);
		//				flag=0;
		//				if (strcmp(&requete,"GET_T")==0) {
		////					printf("ok = GET_T \r\n");
		//					read_temp();
		//					memset(requete, 0, SIZE_REQUETE);
		//				}else if (strcmp(&requete,"GET_P")==0) {
		////					printf("ok = GET_P \r\n");
		//					read_press();
		//					memset(requete, 0, SIZE_REQUETE);
		//				}
		//				else if (strcmp(&requete,"GET_K")==0) {
		////					printf("ok = GET_K \r\n");
		//					memset(requete, 0, SIZE_REQUETE);
		//				}
		//				else if (strcmp(&requete,"GET_A")==0){
		////					printf("ok = GET_A \r\n");
		//					memset(requete, 0, SIZE_REQUETE);
		//				}
		//				else if (strcmp(&requete,"GET_K=1234")==0){
		////					printf("ok = GET_K=1234 \r\n");
		//					memset(requete, 0, SIZE_REQUETE);
		//				}
		//				else{
		//					printf("Requête non reconnue \r\n");
		//					memset(requete, 0, SIZE_REQUETE);
		//				}
		//			}else {
		//				if (flag > sizeof(requete)-1) {
		//					flag=0;
		//					memset(requete, 0, SIZE_REQUETE);
		//				}
		//				requete[flag]=Rx_rasp_to_nucleo;
		//				flag++;
		//				//				if (flag >= 5) {
		//				//					flag=0;
		//				//					printf("Requete trop longue \r\n");
		//				//				}
		//				//				requete[flag]=Rx_rasp_to_nucleo;
		//				//				flag++;
		//			}
		//			//printf("ok");
		//			//			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		//			//			HAL_Delay(500);
		//
		//		}

		//		printf("Data = %x", Rx_rasp_to_nucleo);

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
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 80;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
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
 * @brief CAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN1_Init(void)
{

	/* USER CODE BEGIN CAN1_Init 0 */

	/* USER CODE END CAN1_Init 0 */

	/* USER CODE BEGIN CAN1_Init 1 */

	/* USER CODE END CAN1_Init 1 */
	hcan1.Instance = CAN1;
	hcan1.Init.Prescaler = 16;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_2TQ;
	hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.AutoRetransmission = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN CAN1_Init 2 */

	/* USER CODE END CAN1_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM13 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM13_Init(void)
{

	/* USER CODE BEGIN TIM13_Init 0 */

	/* USER CODE END TIM13_Init 0 */

	/* USER CODE BEGIN TIM13_Init 1 */

	/* USER CODE END TIM13_Init 1 */
	htim13.Instance = TIM13;
	htim13.Init.Prescaler = 9999;
	htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim13.Init.Period = 8000;
	htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM13_Init 2 */

	/* USER CODE END TIM13_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
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
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void)
{

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : Button_Pin */
	GPIO_InitStruct.Pin = Button_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(Button_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LED_Pin */
	GPIO_InitStruct.Pin = LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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

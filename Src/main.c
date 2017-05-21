/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_hal.h"
#include "fatfs.h"
//#include "sdmmc.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_sdram.h"
#include "stm32f769i_discovery_sd.h"
#include "stm32f769i_discovery_qspi.h"
#include "fatfs_storage.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

static HAL_SD_CardCIDTypeDef pSD_CID_Info;
static HAL_SD_CardInfoTypeDef pSD_Card_Info;
volatile HAL_SD_CardStateTypeDef pSD_Card_State;
static QSPI_Info pQSPI_Info;
SD_HandleTypeDef uSdHandle;
SDRAM_HandleTypeDef sdramHandle;

uint8_t *uwInternalBuffer;
#define BASE_ADDRESS       ((uint32_t)0x00000000)
#define SDRAM_BASE_ADDRESS ((uint32_t)0xC0000000)
#define FLASH_BASE_ADDRESS ((uint32_t)0x90000000)

#define seal  FLASH_BASE_ADDRESS
#define sere  ((uint32_t *)0x9011A000)

FATFS SD_FatFs;  /* File system object for SD card logical drive */
char SD_Path[4]; /* SD card logical drive path */
char* pDirectoryFiles[MAX_BMP_FILES];
uint8_t  ubNumberOfFiles = 0;
uint32_t uwBmplen = 0;
uint32_t pDirectoryFileSize[MAX_BMP_FILES];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
static void MPU_Config(void);


/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  MPU_Config();
  uwInternalBuffer = (uint8_t *)SDRAM_BASE_ADDRESS;
  uint32_t counter = 0;
  uint8_t str[30];

  /* USER CODE END 1 */

  /* Enable I-Cache-------------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache-------------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();

  /* USER CODE BEGIN 2 */
  char uart1Data[48] = "Connected to UART One of STM32F769I-Discovery\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)&uart1Data,sizeof(uart1Data), 0xFFFF);
  printf("\r\n");

  BSP_SD_Init();
  BSP_QSPI_Init();
  BSP_SDRAM_Init();

  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);
  BSP_LED_On(LED_GREEN);
  BSP_LED_Off(LED_RED);


  /* SDRAM */
  printf("             SDRAM\r\n");
  printf("----------------------------------\r\n");
  printf(" SDRAM size          : 0x%08lX\r\n", SDRAM_DEVICE_SIZE);
  printf(" MT48LC4M32B2 status : ");
  HAL_SDRAM_StateTypeDef sdramState = HAL_SDRAM_GetState(&sdramHandle);
  switch (sdramState)
  {
  case HAL_SDRAM_STATE_RESET           : printf("RESET\r\n"); break;
  case HAL_SDRAM_STATE_READY           : printf("READY\r\n"); break;
  case HAL_SDRAM_STATE_BUSY            : printf("BUSY\r\n"); break;
  case HAL_SDRAM_STATE_ERROR           : printf("ERROR\r\n"); break;
  case HAL_SDRAM_STATE_WRITE_PROTECTED : printf("WRITE PROTECTED\r\n"); break;
  case HAL_SDRAM_STATE_PRECHARGED      : printf("PRECHARGED\r\n"); break;
  }
  printf("\r\n");


  /* QSPI Flash */
  printf("       QUAD SPI Flash memory\r\n");
  printf("---------------------------------\r\n");
  pQSPI_Info.FlashSize          = (uint32_t)0x00;
  pQSPI_Info.EraseSectorSize    = (uint32_t)0x00;
  pQSPI_Info.EraseSectorsNumber = (uint32_t)0x00;
  pQSPI_Info.ProgPageSize       = (uint32_t)0x00;
  pQSPI_Info.ProgPagesNumber    = (uint32_t)0x00;
  BSP_QSPI_GetInfo(&pQSPI_Info);
  printf(" Flash size         : 0x%08lX\r\n", pQSPI_Info.FlashSize);
  printf(" Erase Sector size  : 0x%08lX\r\n", pQSPI_Info.EraseSectorSize);
  printf(" Erase Sectors      : %ld\r\n", pQSPI_Info.EraseSectorsNumber);
  printf(" Program Page size  : 0x%08lX\r\n", pQSPI_Info.ProgPageSize);
  printf(" Program Pages      : %ld\r\n", pQSPI_Info.ProgPagesNumber);
  printf(" MX25L51245G status : ");
  uint8_t qspires = BSP_QSPI_GetStatus();
  switch (qspires)
  {
  	  case QSPI_OK            : printf("OK\r\n"); break;
  	  case QSPI_ERROR         : printf("ERROR\r\n"); break;
  	  case QSPI_BUSY          : printf("BUSY\r\n"); break;
  	  case QSPI_NOT_SUPPORTED : printf("NOT SUPPORTED\r\n"); break;
  	  case QSPI_SUSPENDED     : printf("SUSPENDED\r\n"); break;
  }
  /*
  //THIS TAKES MINUTES AND MINUTES GO WASH DISHES
  printf("Erasing QSPI Flash...\r\n");
  qspires = BSP_QSPI_Erase_Chip();
  switch (qspires)
  {
  	  case QSPI_OK            : printf(" OK\r\n"); break;
  	  case QSPI_ERROR         : printf(" ERROR\r\n"); break;
  	  case QSPI_BUSY          : printf(" BUSY\r\n"); break;
  	  case QSPI_NOT_SUPPORTED : printf(" NOT SUPPORTED\r\n"); break;
  	  case QSPI_SUSPENDED     : printf(" SUSPENDED\r\n"); break;
  }
  */
  printf("\r\n");


  /* uSD card */
  printf("             MicroSD Card\r\n");
  printf("--------------------------------------\r\n");
  HAL_StatusTypeDef sdres;
  uint8_t isthere;
  isthere = BSP_SD_IsDetected();
  if (isthere == SD_PRESENT)
  {
	  sdres = HAL_SD_GetCardCID(&uSdHandle, &pSD_CID_Info);
	  if (sdres != HAL_OK)
	  {
		  printf("SD GetCardCID failed\r\n");
	  }
	  printf(" Manufacturer     : ");
	  switch (pSD_CID_Info.ManufacturerID)
	  	  {
	  	  	  case 0x00 : printf ("0x00"); break ;
	  	  	  case 0x01 : printf ("0x01"); break ;
	  	  	  case 0x02 : printf ("0x02"); break ;
	  	  	  case 0x03 : printf ("SanDisk"); break ;
	  	  	  case 0x04 : printf ("0x04"); break ;
	  	  	  case 0x05 : printf ("0x05"); break ;
	  	  }
	  printf("\r\n");
	  char *p = (char *)&pSD_CID_Info.ProdName1;
	  printf(" Product name     : %c%c%c%c%c\r\n", p[3], p[2], p[1], p[0], p[4]);
	  //printf(" Revision         : 0x%X\r\n", pSD_CID_Info.ProdRev);
	  //printf(" Serial number    : 0x%08lX\r\n", pSD_CID_Info.ProdSN);
	  //printf(" Manufacture date : 0x%X\r\n", pSD_CID_Info.ManufactDate);

	  sdres = HAL_SD_GetCardInfo(&uSdHandle, &pSD_Card_Info);
	  if (sdres != HAL_OK)
	  {
		  printf("SD GetCardInfo failed\r\n");
	  }
	  printf(" Card type        : ");
	  switch (pSD_Card_Info.CardType)
	  {
	  case CARD_SDSC      : printf("SDSC\r\n"); break;
	  case CARD_SDHC_SDXC : printf("SDHC/SDXC\r\n"); break;
	  case CARD_SECURED   : printf("SECURED\r\n"); break;
	  }
	  //printf(" Version          : %lx\r\n", pSD_Card_Info.CardVersion);
	  //printf(" Class            : %ld\r\n", pSD_Card_Info.Class);
	  //printf(" Block Number     : %ld\r\n", pSD_Card_Info.BlockNbr);
	  //printf(" Block Size       : %ld\r\n", pSD_Card_Info.BlockSize);
	  printf(" Capacity         : %lu bytes\r\n", pSD_Card_Info.BlockNbr * pSD_Card_Info.BlockSize);

	  /*
	  printf(" Status           : ");
	  pSD_Card_State = HAL_SD_GetCardState(&uSdHandle);
	  	  switch(pSD_Card_State)
	  	  {
	  	  	  case HAL_SD_CARD_READY          : printf("READY\r\n"); break;
	  	  	  case HAL_SD_CARD_IDENTIFICATION : printf("IDENTIFICATION\r\n"); break;
	  	  	  case HAL_SD_CARD_STANDBY        : printf("STANDBY\r\n"); break;
	  	  	  case HAL_SD_CARD_TRANSFER       : printf("TRANSFER\r\n"); break;
	  	  	  case HAL_SD_CARD_SENDING        : printf("SENDING\r\n"); break;
	  	  	  case HAL_SD_CARD_RECEIVING      : printf("RECEIVING\r\n"); break;
	  	  	  case HAL_SD_CARD_PROGRAMMING    : printf("PROGRAMMING\r\n"); break;
	  	  	  case HAL_SD_CARD_DISCONNECTED   : printf("DISCONNECTED\r\n"); break;
	  	  	  case HAL_SD_CARD_ERROR          : printf("ERROR\r\n");
	  	   }
	  */
	  printf("\r\n");


	  if(FATFS_LinkDriver(&SD_Driver, SD_Path) == 0)
	  {
	    /* Initialize the Directory Files pointers (heap) ###################*/
	    for (counter = 0; counter < MAX_BMP_FILES; counter++)
	    {
	      pDirectoryFiles[counter] = malloc(MAX_BMP_FILE_NAME);
	      if(pDirectoryFiles[counter] == NULL)
	      {
	        /* Set the Text Color */
	        printf("  Cannot allocate memory ");
	        while(1) { ; }
	      }
	    }

	    /* Get the BMP file names on root directory */
	    ubNumberOfFiles = Storage_GetDirectoryBitmapFiles("/Media", pDirectoryFiles);

	    if (ubNumberOfFiles == 0)
	    {
	      for (counter = 0; counter < MAX_BMP_FILES; counter++)
	      {
	        free(pDirectoryFiles[counter]);
	      }
	      printf("  No Bitmap files...      ");
	      while(1) { ; }
	    }
	  }
	  else
	  {
	    /* FatFs Initialization Error */
	    Error_Handler();
	  }

	  counter = 0;
	  while ((counter) < ubNumberOfFiles)
	  {
		  sprintf ((char*)str, "Media/%-11.11s", pDirectoryFiles[counter]);

		  if (Storage_CheckBitmapFile((const char*)str, &uwBmplen) == 0)
		  	  {
			  	  /* Format the string */
			  	  sprintf ((char*)str, "Media/%-11.11s", pDirectoryFiles[counter]);
			  	  pDirectoryFileSize[counter] = uwBmplen;
			  	  printf(" %ld   %s\r\n", pDirectoryFileSize[counter], str);
		  	  }
	      counter++;
	  }
	  printf("\r\n");

	  uint32_t q = 0x00;
	  uint32_t readres = 0;

	  counter = 0;
	  uint32_t flash_address = BASE_ADDRESS;
	  uint32_t next_address = flash_address;
	  while (counter < ubNumberOfFiles) {
		  q=0;
	  sprintf ((char*)str, "Media/%-11.11s", pDirectoryFiles[counter]);
	  printf(" Loading into SDRAM %s\r\n", str);
	  readres = Storage_OpenReadFile(uwInternalBuffer, (const char*)str);
	  if (readres == 1)
	  {
		  //printf(" OK\r\n");
	  } else {
		  printf(" FAILED\r\n");
	  }

	  printf(" Erasing flash prior to write\r\n");
	  for (q = flash_address; q <= flash_address + pDirectoryFileSize[counter]; q+= 0x1000)
	  {
		  BSP_QSPI_Erase_Block(q);
	  }
	  //printf(" OK\r\n");

	  printf(" Writing at 0x%08lX %s\r\n", (uint32_t)flash_address, (const char*)str);

	  qspires = BSP_QSPI_Write((uint8_t*)uwInternalBuffer, (uint32_t)flash_address, pDirectoryFileSize[counter]);
	  switch (qspires)
	  {
	  	  case QSPI_OK            : break;
	  	  case QSPI_ERROR         : printf(" ERROR\r\n"); break;
	  	  case QSPI_BUSY          : printf(" BUSY\r\n"); break;
	  	  case QSPI_NOT_SUPPORTED : printf(" NOT SUPPORTED\r\n"); break;
	  	  case QSPI_SUSPENDED     : printf(" SUSPENDED\r\n"); break;
	  }

	  printf( " Next available block: 0x%08lX\r\n",q);
	  printf("\r\n");
	  next_address = (q - flash_address);
	  flash_address += next_address;
	  counter++;
	  }




  } else {
	  printf("      MicroSD card not inserted\r\n");
  }
  printf("\r\n");


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Activate the Over-Drive mode 
    */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_SDMMC2
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 3;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART2 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x20020000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

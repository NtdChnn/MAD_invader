/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ILI9341_Touchscreen.h"

#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#include "blank_40.h"
#include "blank_400.h"
#include "fire.h"
#include "monster_1.h"
#include "monster_2.h"
#include "spaceship.h"
#include "monsters_fire.h"
#include "stdlib.h"
#include "EEPROM.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint16_t stage = 0; // 0 = menuScreen , 1 = gameScreen , 2 = pauseScreen
// 3 = winScreen , 4 = loseScreen , 5 = submitScoreScreen , 6 = highScoreScreen
uint16_t posXspace;
uint16_t posYspace;
uint16_t posXbullet = 0;
uint16_t posYbullet = 0;
uint16_t bullet = 0;
uint16_t checkbeep = 0;
uint16_t posXmonster[] = { 50, 150, 250, 100, 200, 100, 200 };
uint16_t posYmonster[] = { 100, 100, 100, 140, 140, 60, 60 };
uint16_t monsterAvaliable[] = { 0, 0, 0, 0, 0, 0, 0 };
uint16_t monsterType[] = { 0, 0, 0, 0, 0, 0, 0 };
uint16_t monsterBullet[] = { 0, 0, 0, 0, 0, 0, 0 };
uint16_t posXmonsterBullet[] = { 0, 0, 0, 0, 0, 0, 0 };
uint16_t posYmonsterBullet[] = { 0, 0, 0, 0, 0, 0, 0 };
uint16_t direction = 0; //0 = left, 1 = right
uint16_t wave;
uint16_t wavestate;
uint16_t score;
uint16_t initGamePage;
uint16_t second;
uint16_t cheatButtonState = 0;
uint16_t pause = 0;
uint16_t initMenu = 1;
uint16_t menuButtonState = 0;

char text[26] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

int playerLen;
uint16_t dataOffset;

uint8_t state = 0;
uint8_t stateChange = 0;

char name[5];

int indexdefault[5] = { 2, 6, 10, 14, 18 };
int playerNumberSort[5][3];

int player1Len;
char player1Name[5];
int player1Score;
int player1Time;

float namelen;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define DEV_ADDR 0xa0
void displayTime(uint16_t second, uint16_t posX, uint16_t posY) {
	uint16_t color = 0xFFFF;
	if (stage != 1) {
		color = 0x0000;
	}
	char str[10];
	sprintf(str, "%.2d:%.2d", second / 60, second % 60);
	ILI9341_Draw_Text(str, posX, posY, color, 2, 0x0000);
}
void displayPlayerTime(uint16_t second, uint16_t posX, uint16_t posY) {
	uint16_t color = 0xFFFF;
	char str[10];
	sprintf(str, "%.2d:%.2d", second / 60, second % 60);
	ILI9341_Draw_Text(str, posX, posY, color, 2, 0x0000);
}
void restart() { //restart game
	wave = 1;
	posXspace = 150;
	posYspace = 220;
	ILI9341_Fill_Screen(0x0000);
	wavestate = 1;
	bullet = 0;
	score = 0;
	initGamePage = 1;
	second = 0;
	ILI9341_Draw_Text("00:00", 200, 0, WHITE, 2, 0x0000);
	ILI9341_Draw_Text("0000", 0, 15, WHITE, 2, 0x0000);
	stage = 1;
}
void gamewin() { //winScreen
	ILI9341_Fill_Screen(0x0000);
	ILI9341_Draw_Text("You Win", 50, 80, WHITE, 5, 0x0000);
	char str[30];
	sprintf(str, "score %.4d", score);
	ILI9341_Draw_Text(str, 90, 130, WHITE, 2, 0x0000);
	sprintf(str, "Time %.2d:%.2d", second / 60, second % 60);
	ILI9341_Draw_Text(str, 90, 150, WHITE, 2, 0x0000);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_RESET);
}
void gamelose() { //loseScreen
	ILI9341_Fill_Screen(0x0000);
	ILI9341_Draw_Text("Game Over", 15, 80, WHITE, 5, 0x0000);
	char str[30];
	sprintf(str, "score %.4d", score);
	ILI9341_Draw_Text(str, 90, 130, WHITE, 2, 0x0000);
	sprintf(str, "Time %.2d:%.2d", second / 60, second % 60);
	ILI9341_Draw_Text(str, 90, 150, WHITE, 2, 0x0000);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_RESET);
}
uint16_t isOverlap(uint16_t posX1, uint16_t posY1, uint16_t posX2,
		uint16_t posY2) { // check is object 20x20 overlap each other
	if (posY1 >= posY2 - 19 && posY1 <= posY2 + 19 && posX1 >= posX2 - 19
			&& posX1 <= posX2 + 19) {
		return 1;
	}
	return 0;
}
void setMonsterAvaliable(uint16_t mon0, uint16_t mon1, uint16_t mon2,
		uint16_t mon3, uint16_t mon4, uint16_t mon5, uint16_t mon6) {
	monsterAvaliable[0] = mon0;
	monsterAvaliable[1] = mon1;
	monsterAvaliable[2] = mon2;
	monsterAvaliable[3] = mon3;
	monsterAvaliable[4] = mon4;
	monsterAvaliable[5] = mon5;
	monsterAvaliable[6] = mon6;
}
void setPosXMonster(uint16_t mon0, uint16_t mon1, uint16_t mon2, uint16_t mon3,
		uint16_t mon4, uint16_t mon5, uint16_t mon6) {
	posXmonster[0] = mon0;
	posXmonster[1] = mon1;
	posXmonster[2] = mon2;
	posXmonster[3] = mon3;
	posXmonster[4] = mon4;
	posXmonster[5] = mon5;
	posXmonster[6] = mon6;
}
void setMonterType(uint16_t mon0, uint16_t mon1, uint16_t mon2, uint16_t mon3,
		uint16_t mon4, uint16_t mon5, uint16_t mon6) {
	monsterType[0] = mon0;
	monsterType[1] = mon1;
	monsterType[2] = mon2;
	monsterType[3] = mon3;
	monsterType[4] = mon4;
	monsterType[5] = mon5;
	monsterType[6] = mon6;
}
void resetMonsterBullet() {
	monsterBullet[0] = 0;
	monsterBullet[1] = 0;
	monsterBullet[2] = 0;
	monsterBullet[3] = 0;
	monsterBullet[4] = 0;
	monsterBullet[5] = 0;
	monsterBullet[6] = 0;
}
uint16_t moveBullet(uint16_t bullet) {
	if (bullet) { //move bullet
		posYbullet -= 1;
		ILI9341_Draw_ImageEx(posXbullet, posYbullet, 4, 10, fire,
		SCREEN_HORIZONTAL_1);

		if (posYbullet == 20) { //check is bullet touch the top edge
			bullet = 0;
			ILI9341_Draw_ImageEx(posXbullet, posYbullet, 4, 10, blank_40,
			SCREEN_HORIZONTAL_1);
		}
	}
	return bullet;
}
uint16_t moveMonsterBullet(uint16_t monsterBullet, uint16_t i) {
	if (monsterBullet) { //move monsterBullet
		posYmonsterBullet[i] += 1;
		ILI9341_Draw_ImageEx(posXmonsterBullet[i], posYmonsterBullet[i], 4, 10,
				monsters_fire, SCREEN_HORIZONTAL_1);

		if (posYmonsterBullet[i] >= 230) { //check is monsterBullet touch the buttom edge
			monsterBullet = 0;
			ILI9341_Draw_ImageEx(posXmonsterBullet[i], posYmonsterBullet[i], 4,
					10, blank_40, SCREEN_HORIZONTAL_1);
		}
	} else {
		ILI9341_Draw_ImageEx(posXmonsterBullet[i], posYmonsterBullet[i], 4, 10,
				blank_40, SCREEN_HORIZONTAL_1);
	}
	return monsterBullet;
}
void controlSpaceship() {
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET) { //move right
		if (posXspace < 300) {
			posXspace += 1;
		}
	}
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET) { //move left
		if (posXspace > 0) {
			posXspace -= 1;
		}
	}
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET) { //move up
		if (posYspace > 20) {
			posYspace -= 1;
		}
	}
	if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_RESET) { //move down
		if (posYspace < 220) {
			posYspace += 1;
		}
	}
	if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_RESET
			|| HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_2) == GPIO_PIN_RESET) { //shoot bullet
		if (bullet == 0) {
			posXbullet = posXspace + 8;
			posYbullet = posYspace - 10;
			bullet = 1;
		}
	}
}

static const size_t NUM_COLS = 3;
/* Lexicographically compare two arrays of size NUM_COLS. */
int CompareArrays(const void *arr1, const void *arr2) {
	/* Convert back to the proper type. */
	int one = *((int*) arr1); // score arr1
	int two = *((int*) arr2); // score arr2
	int oneone = *((int*) arr1 + 1); // time arr1
	int twotwo = *((int*) arr2 + 1); // time arr2
	/* Do an element-by-element comparison.  If a mismatch is found, report how
	 * the arrays compare against one another.
	 */
	//Max -> Min (score compare)
	if (one > two)
		return -1;
	if (one < two)
		return +1;
	/* If we get here, the arrays are equal to one another. */
	//Min -> Max (time compare)
	if (oneone < twotwo)
		return -1;
	if (oneone > twotwo)
		return +1;
	return 0;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

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
	MX_USART3_UART_Init();
	MX_SPI5_Init();
	MX_TIM1_Init();
	MX_RNG_Init();
	MX_I2C1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */
	ILI9341_Init(); //initial driver setup to drive ili9341
	ILI9341_Fill_Screen(BLACK);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);
	//Clear EEPROM
//	for (int i = 0; i < 22; i++) {
//	EEPROM_PageErase(i);
//	}
//	float dataw = 0;
//	EEPROM_Write_NUM (1, 0, dataw);
	playerLen = EEPROM_Read_NUM(1, 0);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//-----------------------------------------------------------------------------menu
		if (stage == 0) {
			int select; // 0 = start, 1 = score
			if (initMenu) {
				ILI9341_Fill_Screen(BLACK);
				ILI9341_Draw_Text("MAD", 117, 30, WHITE, 5, BLACK);
				ILI9341_Draw_Text("Invaders", 65, 70, WHITE, 4, BLACK);
				ILI9341_Draw_Text("Start", 115, 140, YELLOW, 3, BLACK);
				ILI9341_Draw_Text("Score", 115, 170, WHITE, 3, BLACK);
				initMenu = 0;
				menuButtonState = 1;
				select = 0;
			}
			if (menuButtonState == 0
					&& (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET
							|| HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3)
									== GPIO_PIN_RESET)) {
				menuButtonState = 1;
				if (select) {
					ILI9341_Draw_Text("Start", 115, 140, YELLOW, 3, BLACK);
					ILI9341_Draw_Text("Score", 115, 170, WHITE, 3, BLACK);
					select = 0;
				} else {
					ILI9341_Draw_Text("Start", 115, 140, WHITE, 3, BLACK);
					ILI9341_Draw_Text("Score", 115, 170, YELLOW, 3, BLACK);
					select = 1;
				}
			}
			if (menuButtonState == 1
					&& HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_SET
					&& HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_SET
					&& HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_SET) {
				menuButtonState = 0;
			}
			if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_SET
					&& HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_SET
					&& HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_RESET
					&& menuButtonState == 0) {
				if (select) {
					ILI9341_Fill_Screen(BLACK);
					stage = 6;
				} else {
					restart();
				}
			}
		} else
		//-----------------------------------------------------------------------------game screen
		if (stage == 1) {

			if (initGamePage) {
				second = 0;
				ILI9341_Draw_Text("0000", 0, 15, WHITE, 2, 0x0000);
				ILI9341_Draw_Text("00:00", 200, 0, WHITE, 2, 0x0000);
				initGamePage = 0;
			}
			char str[30];
			if (pause == 1) {
				pause = 0;
				ILI9341_Fill_Screen(BLACK);
				sprintf(str, "Wave : %d", wave);
				ILI9341_Draw_Text(str, 0, 0, WHITE, 2, 0x0000); //draw level
				sprintf(str, "%.4d", score);
				ILI9341_Draw_Text(str, 0, 15, WHITE, 2, 0x0000);
				displayTime(second, 200, 0);
			}
			if (wavestate) {
				ILI9341_Draw_Rectangle(0, 50, 320, 100, 0x0000);
				if (wave == 7) {
					stage = 3;
				}
				sprintf(str, "Wave : %d", wave);
				ILI9341_Draw_Text(str, 0, 0, WHITE, 2, 0x0000); //draw level
				if (wave == 1) {
					setMonsterAvaliable(1, 1, 1, 0, 0, 0, 0);
					setMonterType(2, 2, 2, 0, 0, 0, 0);
					resetMonsterBullet();
				} else if (wave == 2) {
					setMonsterAvaliable(1, 1, 1, 0, 0, 0, 0);
					setMonterType(2, 1, 2, 0, 0, 0, 0);
					resetMonsterBullet();
				} else if (wave == 3) {
					setMonsterAvaliable(1, 1, 1, 1, 1, 0, 0);
					setMonterType(2, 2, 2, 1, 1, 0, 0);
					resetMonsterBullet();
				} else if (wave == 4) {
					setMonsterAvaliable(1, 1, 1, 1, 1, 0, 0);
					setMonterType(1, 1, 1, 2, 2, 0, 0);
					resetMonsterBullet();
				} else if (wave == 5) {
					setMonsterAvaliable(1, 1, 1, 1, 1, 1, 1);
					setMonterType(2, 2, 2, 1, 1, 1, 1);
					resetMonsterBullet();
				} else if (wave == 6) {
					setMonsterAvaliable(1, 1, 1, 1, 1, 1, 1);
					setMonterType(1, 1, 1, 1, 1, 1, 1);
					resetMonsterBullet();
				}
				setPosXMonster(50, 150, 250, 100, 200, 100, 200);
				wavestate = 0;
			}
			for (int i = 0; i < 7; i++) { //everything about monster

				if (monsterAvaliable[i]) {
					if ((posYbullet > posYmonster[i] - 4)
							&& (posYbullet < posYmonster[i] + 19)
							&& (posXbullet > posXmonster[i] - 4)
							&& (posXbullet < posXmonster[i] + 19)
							&& bullet == 1) { //check is bullet touch monster
						bullet = 0;
						ILI9341_Draw_ImageEx(posXbullet, posYbullet, 4, 10,
								blank_40, SCREEN_HORIZONTAL_1); //delete bullet
						monsterAvaliable[i] = 0;
						checkbeep = 1;
						if (monsterType[i] == 2) {
							score += 35;
						} else {
							score += 65;
						}
						sprintf(str, "%.4d", score);
						ILI9341_Draw_Text(str, 0, 15, WHITE, 2, 0x0000);

						if (monsterAvaliable[0] == 0 && monsterAvaliable[1] == 0
								&& monsterAvaliable[2] == 0
								&& monsterAvaliable[3] == 0
								&& monsterAvaliable[4] == 0
								&& monsterAvaliable[5] == 0
								&& monsterAvaliable[6] == 0) { //check is all monster die
							wave++;
							wavestate = 1;
						}
					}

					if (isOverlap(posXspace, posYspace, posXmonster[i],
							posYmonster[i])) { //check is spaceship touch monster
						ILI9341_Draw_Rectangle(0, 50, 320, 100, 0x0000);
						stage = 4;
					}

					if (monsterType[i] == 2) {
						ILI9341_Draw_ImageEx(posXmonster[i], posYmonster[i], 20,
								20, monster_2, SCREEN_HORIZONTAL_1); //draw monster 1
					} else { //monstertype == 1
						ILI9341_Draw_ImageEx(posXmonster[i], posYmonster[i], 20,
								20, monster_1, SCREEN_HORIZONTAL_1); //draw monster 2
						if (rand() % 50 == 0 && monsterBullet[i] == 0) {
							posXmonsterBullet[i] = posXmonster[i] + 8;
							posYmonsterBullet[i] = posYmonster[i] + 19;
							monsterBullet[i] = 1;
						}
					}

				} else {
					ILI9341_Draw_ImageEx(posXmonster[i], posYmonster[i], 20, 20,
							blank_400, SCREEN_HORIZONTAL_1); //delete monster
				}

				monsterBullet[i] = moveMonsterBullet(monsterBullet[i], i);
				if (monsterBullet[i] && posXmonsterBullet[i] >= posXspace - 4
						&& posXmonsterBullet[i] <= posXspace + 19
						&& posYmonsterBullet[i] >= posYspace - 10
						&& posYmonsterBullet[i] <= posYspace + 10) { //check is monsterBullet touch spaceship
					ILI9341_Draw_Rectangle(0, 50, 320, 100, 0x0000);
					stage = 4;
				}
				ILI9341_Draw_ImageEx(posXspace, posYspace, 20, 20, spaceship,
				SCREEN_HORIZONTAL_1); // draw spaceship

				if (direction) {
					if (i < 3) {
						//move right
						posXmonster[i] += 1;
					} else {
						//move left
						posXmonster[i] -= 1;
					}
				} else {
					if (i < 3) {
						//move left
						posXmonster[i] -= 1;
					} else {
						//move right
						posXmonster[i] += 1;
					}
				}

			}

			if (posXmonster[0] == 70) {
				direction = 0; // move left
			} else if (posXmonster[0] == 30) {
				direction = 1; // move right
			}

			if (checkbeep) { //checkbeep
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_SET);
				checkbeep = 0;
			} else {
				HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_RESET);
			}

			if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_RESET
					&& HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_2) == GPIO_PIN_RESET
					&& cheatButtonState == 0) { //cheat
				wave++;
				wavestate = 1;
				cheatButtonState = 1;
			}
			if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_SET
					&& HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_2) == GPIO_PIN_SET
					&& cheatButtonState == 1) {
				cheatButtonState = 0;
			}
		} else
		//----------------------------------------------------------pause
		if (stage == 2) {
			int select; //0 = resume, 1 = restart , 2 = menu
			int buttonState;
			if (pause == 0) {
				ILI9341_Fill_Screen(0x0000);
				ILI9341_Draw_Text("PAUSE", 85, 40, WHITE, 5, 0x0000);
				ILI9341_Draw_Text("resume", 105, 100, YELLOW, 3, BLACK);
				ILI9341_Draw_Text("restart", 95, 130, WHITE, 3, BLACK);
				ILI9341_Draw_Text("menu", 120, 160, WHITE, 3, BLACK);
				pause = 1;
				select = 0;
				buttonState = 0;
			}
			if (buttonState == 0
					&& (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET
							|| HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3)
									== GPIO_PIN_RESET)) {
				buttonState = 1;
				if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET) {
					if (select != 0) {
						select--;
					}
				} else {
					if (select != 2) {
						select++;
					}
				}

				if (select == 0) {
					ILI9341_Draw_Text("resume", 105, 100, YELLOW, 3, BLACK);
					ILI9341_Draw_Text("restart", 95, 130, WHITE, 3, BLACK);
					ILI9341_Draw_Text("menu", 120, 160, WHITE, 3, BLACK);
				} else if (select == 1) {
					ILI9341_Draw_Text("resume", 105, 100, WHITE, 3, BLACK);
					ILI9341_Draw_Text("restart", 95, 130, YELLOW, 3, BLACK);
					ILI9341_Draw_Text("menu", 120, 160, WHITE, 3, BLACK);
				} else if (select == 2) {
					ILI9341_Draw_Text("resume", 105, 100, WHITE, 3, BLACK);
					ILI9341_Draw_Text("restart", 95, 130, WHITE, 3, BLACK);
					ILI9341_Draw_Text("menu", 120, 160, YELLOW, 3, BLACK);
				}
			}
			if (buttonState == 1
					&& HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_SET
					&& HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3) == GPIO_PIN_SET) {
				buttonState = 0;
			}
			if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) == GPIO_PIN_RESET) {
				ILI9341_Fill_Screen(BLACK);
				if (select == 0) {
					stage = 1;
				} else if (select == 1) {
					restart();
				} else if (select == 2) {
					stage = 0;
					initMenu = 1;
				}
			}
		} else
		//----------------------------------------------------------win Screen
		if (stage == 3) {
			gamewin();
			HAL_Delay(4900);
			ILI9341_Fill_Screen(BLACK);
			playerLen = EEPROM_Read_NUM(1, 0);
			if (playerLen < 5) {
				stage = 5;
			} else if (playerLen == 5) {
				for (int i = 0; i < playerLen; i++) {
					int indexshow = (i * 4) + 2;
					playerNumberSort[i][0] = EEPROM_Read_NUM(indexshow + 2, 0);
					playerNumberSort[i][1] = EEPROM_Read_NUM(indexshow + 3, 0);
					playerNumberSort[i][2] = indexdefault[i];
				}

				qsort((const int*) &playerNumberSort, playerLen,
						sizeof(int[NUM_COLS]), CompareArrays);

				if (score > playerNumberSort[4][0]
						|| (second < playerNumberSort[4][1]
								&& score == playerNumberSort[4][0])) {
					stage = 5;
				} else {
					ILI9341_Fill_Screen(BLACK);
					stage = 0;
					initMenu = 1;
				}
			} else {
				ILI9341_Fill_Screen(BLACK);
				stage = 0;
				initMenu = 1;
			}
		} else
		//----------------------------------------------------------lose Screen
		if (stage == 4) {
			gamelose();
			HAL_Delay(4900);
			ILI9341_Fill_Screen(BLACK);
			playerLen = EEPROM_Read_NUM(1, 0);
			if (playerLen < 5) {
				stage = 5;
			} else if (playerLen == 5) {
				for (int i = 0; i < playerLen; i++) {
					int indexshow = (i * 4) + 2;
					playerNumberSort[i][0] = EEPROM_Read_NUM(indexshow + 2, 0);
					playerNumberSort[i][1] = EEPROM_Read_NUM(indexshow + 3, 0);
					playerNumberSort[i][2] = indexdefault[i];
				}

				qsort((const int*) &playerNumberSort, playerLen,
						sizeof(int[NUM_COLS]), CompareArrays);

				if (score > playerNumberSort[4][0]
						|| (second < playerNumberSort[4][1]
								&& score == playerNumberSort[4][0])) {
					stage = 5;
				} else {
					ILI9341_Fill_Screen(BLACK);
					stage = 0;
					initMenu = 1;
				}
			} else {
				ILI9341_Fill_Screen(BLACK);
				stage = 0;
				initMenu = 1;
			}
		}
		//----------------------------------------------------------submit Screen
		if (stage == 5) {

			uint8_t numx = 15;
			uint8_t numy = 120;
			uint8_t num2x = 12;
			uint8_t num2y = 136;
			uint8_t check = 0;
			uint8_t num3x = 40;
			memset(name, 0, strlen(name)); //Clear name
			char temp[1];

			int dataTemp;

			//Draw text
			for (int i = 0; i < 26; i++) {
				char str[100];
				sprintf(str, "%c", text[i]);
				if (i == 10 || i == 20) {
					numx = 15;
					numy += 30;
				}
				ILI9341_Draw_Text(str, numx, numy, WHITE, 2, BLACK);
				numx += 25;
			}

			//Draw cursor
			ILI9341_Draw_Rectangle(num2x, num2y, 16, 4, WHITE);

			while (1) {
				HAL_Delay(100);
				//Move right
				if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET) {
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, BLACK);
					if (check == 9 || check == 19) {
						num2x = 12;
						num2y += 30;
						check += 1;
					} else if (check == 25) {
						num2x = 12;
						num2y = 136;
						check = 0;
					} else {
						num2x += 25;
						check += 1;
					}
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, WHITE);
					//Move left
				} else if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0)
						== GPIO_PIN_RESET) {
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, BLACK);
					if (check == 10 || check == 20) {
						num2x = 237;
						num2y -= 30;
						check -= 1;
					} else if (check == 0) {
						num2x = 137;
						num2y = 196;
						check = 25;
					} else {
						num2x -= 25;
						check -= 1;
					}
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, WHITE);

				} else if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3)
						== GPIO_PIN_RESET) {
					//Move top
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, BLACK);
					if (check <= 5) {
						num2y += 60;
						check += 20;
					} else if (check <= 9) {
						num2y += 30;
						check += 10;
					} else {
						num2y -= 30;
						check -= 10;
					}
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, WHITE);
				} else if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_3)
						== GPIO_PIN_RESET) {
					//Move bottom
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, BLACK);
					if (check >= 20) {
						num2y -= 60;
						check -= 20;
					} else if (check >= 16) {
						num2y -= 30;
						check -= 10;
					} else {
						num2y += 30;
						check += 10;
					}
					ILI9341_Draw_Rectangle(num2x, num2y, 16, 5, WHITE);

				} else if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1)
						== GPIO_PIN_RESET) { //select
					//Select character
					if (strlen(name) < 5) {
						char str[100];
						sprintf(str, "%c", text[check]);
						ILI9341_Draw_Text(str, num3x, 80, WHITE, 2,
						BLACK);
						num3x += 12;
						sprintf(temp, "%c", text[check]);
						strcat(name, temp);
					}

				} else if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0)
						== GPIO_PIN_RESET) { //pause
					//					delete name
					if (strlen(name) > 0) {
						num3x -= 12;
						ILI9341_Draw_Rectangle(num3x, 80, 12, 20,
						BLACK);
						name[strlen(name) - 1] = 0;
					}
				} else if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_RESET
						|| HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_2) == GPIO_PIN_RESET) { //A,B
						//submit name
					playerLen = EEPROM_Read_NUM(1, 0);
					if (playerLen < 5) {
						namelen = strlen(name);
						int index = (playerLen * 4) + 2;
						EEPROM_Write_NUM(index, 0, namelen);
						EEPROM_Write(index + 1, 0, name, strlen(name));
						EEPROM_Write_NUM(index + 2, 0, score);
						EEPROM_Write_NUM(index + 3, 0, second);
						dataTemp = playerLen + 1;
						EEPROM_Write_NUM(1, 0, dataTemp);
					}
					if (playerLen == 5) {
						for (int i = 0; i < playerLen; i++) {
							int indexshow = (i * 4) + 2;
							playerNumberSort[i][0] = EEPROM_Read_NUM(
									indexshow + 2, 0);
							playerNumberSort[i][1] = EEPROM_Read_NUM(
									indexshow + 3, 0);
							playerNumberSort[i][2] = indexdefault[i];
						}

						qsort((const int*) &playerNumberSort, playerLen,
								sizeof(int[NUM_COLS]), CompareArrays);

						if (score > playerNumberSort[4][0]
								|| (second < playerNumberSort[4][1]
										&& score == playerNumberSort[4][0])) {
							namelen = strlen(name);
							EEPROM_Write_NUM(playerNumberSort[4][2], 0,
									namelen);
							EEPROM_Write(playerNumberSort[4][2] + 1, 0, name,
									strlen(name));
							EEPROM_Write_NUM(playerNumberSort[4][2] + 2, 0,
									score);
							EEPROM_Write_NUM(playerNumberSort[4][2] + 3, 0,
									second);
						}
					}
					break;
				}
			}

			ILI9341_Fill_Screen(BLACK);
			stage = 0;
			initMenu = 1;
		}
		//---------------------------------------------------------------------High score
		if (stage == 6) {
			uint8_t num4y = 60;
			playerLen = EEPROM_Read_NUM(1, 0);

			for (int i = 0; i < playerLen; i++) {
				int indexshow = (i * 4) + 2;
				playerNumberSort[i][0] = EEPROM_Read_NUM(indexshow + 2, 0);
				playerNumberSort[i][1] = EEPROM_Read_NUM(indexshow + 3, 0);
				playerNumberSort[i][2] = indexdefault[i];
			}

			qsort((const int*) &playerNumberSort, playerLen,
					sizeof(int[NUM_COLS]), CompareArrays);

			ILI9341_Draw_Text("Name", 45, 25, WHITE, 2, BLACK);
			ILI9341_Draw_Text("Score", 120, 25, WHITE, 2, BLACK);
			ILI9341_Draw_Text("Time", 200, 25, WHITE, 2, BLACK);

			//Draw player leaderboard
			for (int i = 0; i < playerLen; i++) {
				memset(player1Name, 0, strlen(player1Name));
				player1Len = EEPROM_Read_NUM(playerNumberSort[i][2], 0);
				EEPROM_Read(playerNumberSort[i][2] + 1, 0, player1Name,
						player1Len);
				player1Score = EEPROM_Read_NUM(playerNumberSort[i][2] + 2, 0);
				player1Time = EEPROM_Read_NUM(playerNumberSort[i][2] + 3, 0);

				char str[10];
				sprintf(str, "%d.", i + 1);
				ILI9341_Draw_Text(str, 25, num4y, WHITE, 2, BLACK);
				ILI9341_Draw_Text(player1Name, 45, num4y, WHITE, 2,
				BLACK);
				char str1[10];
				sprintf(str1, "%.4d", player1Score);
				ILI9341_Draw_Text(str1, 120, num4y, WHITE, 2, BLACK);
				displayPlayerTime(player1Time, 200, num4y);
				num4y += 35;
			}
			while (1) {
				if (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_5) == GPIO_PIN_RESET
						|| HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_2) == GPIO_PIN_RESET)				//A,B
								{
					stage = 0;
					ILI9341_Fill_Screen(BLACK);
					initMenu = 1;
					break;
				}
			}

		}
	}

	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 216;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_0) {
		if (stage == 1) {
			stage = 2;
		}
	}
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

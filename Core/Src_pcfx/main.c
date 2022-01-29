/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#include <stdint.h>
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "buttons.h"
#include "lcd.h"
/* USER CODE END Includes */

extern int mytoupper(int c);
void strupr (char* s)
{
    while (*s) { *s = mytoupper(*s); s++; }
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	lcd_init();
	D_DoomMain();
	lcd_exit();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* Required stubs for it to compile (due to issues with libc on PCFX for whatever reason) */

// Stub for `kill`
int kill(int pid, int sig) {
    return -1;
}

// Stub for `getpid`
int getpid(void) {
    // Return a dummy process ID
    return 1;
}

// Stub for `write`
int write(int fd, const void *buf, int count) {
    // Simulate successful write (no actual output)
    return 0;
}

// Stub for `close`
int close(int fd) {
    // Simulate successful close
    return 0;
}

// Stub for `lseek`
int lseek(int fd, int offset, int whence) {
    return -1;
}

// Stub for `read`
int read(int fd, void *buf, int count) {
    // Simulate an end-of-file or error
    return 0;
}

// Stub for `fstat`
int fstat(int fd, struct stat *st) {
    return -1;
}
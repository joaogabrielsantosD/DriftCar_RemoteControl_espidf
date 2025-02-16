#ifndef HARDWARE_DEFS_H
#define HARDWARE_DEFS_H

#include <driver/gpio.h>

#define println() (printf("\r\n"))
#define delay(ms) (vTaskDelay(pdMS_TO_TICKS(ms)))

/* Logic Levels */
#define LOW  0
#define HIGH 1

/* LDC 128x64 pins */
#define SCL_LCD_PIN GPIO_NUM_22
#define SDA_LCD_PIN GPIO_NUM_21

/* Potenciometer (Analog) Read commands */
#define ANALOG_STEERINGWHELL_PIN GPIO_NUM_33
#define ANALOG_MOTOR_SPEED_PIN   GPIO_NUM_32

/* Push Button (Digital) Read commands */
#define LEFT_COMMAND_PIN      GPIO_NUM_4
#define RIGHT_COMMAND_PIN     GPIO_NUM_5
#define CONFIRM_COMMAND_PIN   GPIO_NUM_15
#define HEADLIGHT_COMMAND_PIN GPIO_NUM_19

/* Analog pin to read the voltage */
#define SOC_PIN               GPIO_NUM_34

#endif
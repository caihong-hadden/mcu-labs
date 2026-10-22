#ifndef __MARIO_H
#define __MARIO_H

#include "main.h"

// Keys definitions: Active Low (Pressed = 0)
// PA0: Jump, PB12: Left, PB13: Right, PB9: Action/Restart
#define KEY_JUMP_PIN    GPIO_PIN_0
#define KEY_JUMP_PORT   GPIOA

#define KEY_LEFT_PIN    GPIO_PIN_12
#define KEY_LEFT_PORT   GPIOB

#define KEY_RIGHT_PIN   GPIO_PIN_13
#define KEY_RIGHT_PORT  GPIOB

#define KEY_ACTION_PIN  GPIO_PIN_9
#define KEY_ACTION_PORT GPIOB

#define BUZZER_PIN      GPIO_PIN_7
#define BUZZER_PORT     GPIOA

#define LED_READY_PIN   GPIO_PIN_0
#define LED_BEAT_PIN    GPIO_PIN_1
#define LED_PORT        GPIOB

void Mario_Game_Init(void);
void Mario_Game_Update(void);

#endif /* __MARIO_H */

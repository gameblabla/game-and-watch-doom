#include <stdint.h>
#include "main.h"
// HAL_GPIO_ReadPin
// #define BTN_PAUSE_Pin GPIO_PIN_13
// #define BTN_PAUSE_GPIO_Port GPIOC
// #define BTN_GAME_Pin GPIO_PIN_1
// #define BTN_GAME_GPIO_Port GPIOC
// #define BTN_TIME_Pin GPIO_PIN_4
// #define BTN_TIME_GPIO_Port GPIOC
// #define BTN_A_Pin GPIO_PIN_9
// #define BTN_A_GPIO_Port GPIOD
// #define BTN_Left_Pin GPIO_PIN_11
// #define BTN_Left_GPIO_Port GPIOD
// #define BTN_Down_Pin GPIO_PIN_14
// #define BTN_Down_GPIO_Port GPIOD
// #define BTN_Right_Pin GPIO_PIN_15
// #define BTN_Right_GPIO_Port GPIOD
// #define BTN_Up_Pin GPIO_PIN_0
// #define BTN_Up_GPIO_Port GPIOD
// #define BTN_B_Pin GPIO_PIN_5
// #define BTN_B_GPIO_Port GPIOD
#include <stdbool.h>
#define B_Left (1 << 0)
#define B_Up (1 << 1)
#define B_Right (1 << 2)
#define B_Down (1 << 3)
#define B_A (1 << 4)
#define B_B (1 << 5)
#define B_C (1 << 6)
#define B_D (1 << 7)
#define B_START (1 << 8)

short buttons_get() {
	uint32_t padtype, paddata;
	padtype = eris_pad_type(0);
	/* Do it twice due to liberis bug */
	paddata = eris_pad_read(0);
	paddata = eris_pad_read(0);
	
	bool left = (paddata & (1 << 11)) ? 1 : 0;
    bool right = (paddata & (1 << 9)) ? 1 : 0;
    bool up = (paddata & (1 << 8)) ? 1 : 0;
    bool down = (paddata & (1 << 10)) ? 1 : 0;
    bool a = (paddata & (1 << 0)) ? 1 : 0;
    bool b = (paddata & (1 << 1)) ? 1 : 0;
    bool c = (paddata & (1 << 2)) ? 1 : 0;
    bool d = (paddata & (1 << 3)) ? 1 : 0;
    bool start = (paddata & (1 << 7)) ? 1 : 0;

    return (
        left | (up << 1) | (right << 2) | (down << 3) | (a << 4) | (b << 5) | (c << 6) | (d << 7) | (start << 8)
    );
}

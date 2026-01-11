#ifndef KEY_H
#define KEY_H
#include <stdint.h>

#include "stm32f103xb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */

/* Defines */


/* Typedef */
typedef struct {
    GPIO_TypeDef  *GPIOx;      // Key port
    uint32_t pin;        // Key pin
    uint8_t flag;      // Key state flag
    uint8_t time_stamp; // Time stamp for debouncing
    uint8_t time_cnt;   // Time counter 
} key_s;
//我key_s结构体里的flag是一个8位标志位，从低到高分别是：当前引脚电平、前次引脚电平、处于周期内标志位、按下键标志、我key_s结构体里的flag是一个8位标志位，从低到高分别是：当前引脚电平、前次引脚电平、处于周期内标志位、按下键标志、
/* Function Declarations */
void key_init(key_s* key);
void key_scan(key_s* key);
uint8_t key_get_state(key_s* key);

#ifdef __cplusplus
}
#endif
#endif /* KEY_H */
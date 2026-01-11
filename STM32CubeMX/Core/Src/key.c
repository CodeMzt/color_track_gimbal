/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key.c
  * @brief   key module implementation
  * @author  Mozart
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes */
#include "key.h"

#include <stddef.h>

#include "stdint.h"
#include "stm32f1xx_hal.h"

#define KEY_CIRCLE_TIME 20    // Key scan cycle time


/**
 * @brief  Initialize key structure
 * @param  key: Pointer to key structure
 * @retval None
 */
void key_init(key_s* key)
{
    if (key == NULL) {
        return;
    }
    
    key->flag = 0x03;// Assuming pull-up resistor, default state is HIGH
    key->time_stamp = 0;
    key->time_cnt = 0;
}

/**
 * @brief  Scan key state
 * @param  key: Pointer to key structure
 * @retval None
 */
void key_scan(key_s* key)
{
    if (key == NULL) {
        return;
    }
    key->time_cnt += key->flag>>2 & 0x01; // Increment time counter if in cycle
    if (key->flag & 0x01)key->flag |= 0x02; // Store previous state
    else key->flag &= 0xFD;
    // Read current key state
    if (HAL_GPIO_ReadPin(key->GPIOx, key->pin))key->flag |= 1;
    else key->flag &= 0xFE;
    
    if (key->flag & 0x02 && !(key->flag & 0x01)) { // State changed from HIGH to LOW, possible key press
        if (!(key->flag&0x04)) { // Not already in cycle
            key->flag |= 0x04; // Set in cycle flag
            key->time_cnt = 0; // Reset time counter
            key->time_stamp=0;
        }else { // Already in cycle

        }
    }
    else if (key->flag & 0x01 && !(key->flag & 0x02)) { // State changed from LOW to HIGH, possible key release
        if (key->flag & 0x04) { // Was in cycle
            if (!key->time_stamp) {// First time stamp
                if (key->time_cnt>=KEY_CIRCLE_TIME*0.8) { /** long press **/
                    key->time_cnt = 0; // Reset time counter
                    key->time_stamp = 0; // Reset time stamp
                    key->flag |= 0x23; // Set long press flag
                    key->flag &= 0x23; // Clear flag except long press flag
                }else {
                    key->flag |= 0x08; // Set short press flag
                    key->time_stamp = key->time_cnt; // Record time stamp
                }
            }else {// Second time stamp
                if (key->time_cnt - key->time_stamp <= KEY_CIRCLE_TIME*0.8) { /** double press **/
                    key->time_cnt = 0; // Reset time counter
                    key->time_stamp = 0; // Reset time stamp
                    key->flag |= 0x13; // Set double press flag
                    key->flag &= 0x13; // Clear flag except double press flag
                }
            }
        }else {/** long press **/
            key->time_cnt = 0; // Reset time counter
            key->time_stamp = 0; // Reset time stamp
            key->flag |= 0x23; // Set long press flag
            key->flag &= 0x23; // Clear flag except long press flag
        }
    }
    if (key->time_cnt >= KEY_CIRCLE_TIME) {
        if (key->flag&0x08&&!(key->flag&0x10)&&!(key->flag&0x20)) { // Short press timeout
            key->flag |= 0x40; // Set short press processed flag
        }
        key->flag &= 0x73; // Clear cycle flags, keep only press flags
        key->time_cnt = 0; // Reset time counter
        key->time_stamp = 0; // Reset time stamp
    }
}

/**
 * @brief  Get key state
 * @param  key: Pointer to key structure
 * @return Key state
 * @retval 2: long press, 1: double press, 4: short press, 0: no press
 */
uint8_t key_get_state(key_s* key)
{
    uint8_t state;
    if (key == NULL) {
        return 0;
    }
    state = key->flag>>4 ;
    key->flag &= 0x0F ;
    return state;
}

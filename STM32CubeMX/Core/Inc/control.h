//
// Created by Mozart on 2026/1/11.
//

#ifndef GIMBAL_CONTROL_H
#define GIMBAL_CONTROL_H
#include "stm32f1xx_hal.h"

int control(UART_HandleTypeDef *huart) ;
void control_set_target(float x,float y);

#endif //GIMBAL_CONTROL_H
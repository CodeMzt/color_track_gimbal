//
// Created by Mozart on 2025/11/8.
//

#ifndef OLED_H
#define OLED_H
#include <stdint.h>

void OLED_Init(void) ;
void OLED_ShowChar(uint8_t x,uint8_t y,char Char);
void OLED_Clear();
void OLED_ShowString(uint8_t x,uint8_t y,const char* Str,uint8_t Len) ;
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t Num,uint8_t Len);
void OLED_ShowSignNum(uint8_t x,uint8_t y,int32_t Num,uint8_t Len);

#endif //OLED_H
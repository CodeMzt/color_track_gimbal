/********************************************************************************

* @File MyI2C.c

* @Author: Ma Ziteng

* @Version: 1.0

* @Date: 2025-11

* @Description: 基于HAL库的延时

********************************************************************************/

#include "delay.h"
#include "stm32f103xb.h"
#include "core_cm3.h"
#include "stdint.h"

/**
  * @brief		利用系统定时器实现微秒级延时
  * @param		usdelay 期望延时微秒数
  * @return  	无
  * @note		操作寄存器
  */
void Delay_us(uint32_t usdelay) {
    SysTick->LOAD = (72 * usdelay); //定时器重装值
    SysTick->VAL = 0;       //定时器清零
    /**CTRL寄存器描述
    *位	名称	描述
    *位 0	    ENABLE	    使能 SysTick 定时器。
    *位 1	    TICKINT	    使能 SysTick 异常（中断）请求。
    *位 2	    CLKSOURCE	SysTick 时钟源选择：0 = 外部时钟八分频（HCLK/8），1 = 处理器时钟（SYSCLK）。
    *位 3-15	保留	        未来使用保留位。
    *位 16	    COUNTFLAG	如果自 SysTick 计时器上次读取此寄存器以来计数为 0，则返回 1，否则返回 0。
    *位 17-31	保留	        未来使用保留位。
    * 0000 0000 0000 0000 0000 0000 0000 0101
     */
    SysTick->CTRL = 0x00000005;
    while (!(SysTick->CTRL&0x00010000));
    SysTick->CTRL = 4;
}

/**
  * @brief		毫秒级延时
  * @param		mdelay 期望延时微秒数
  * @return  	无
  * @note		调用Delay_us
  */
void Delay_ms(uint32_t msdelay) {
    while (msdelay--) {Delay_us(1000);}
}

/**
  * @brief		毫秒级延时
  * @param		sdelay 期望延时微秒数
  * @return  	无
  * @note		调用Delay_us
  */
void Delay_s(uint32_t sdelay) {
    while (sdelay--) {Delay_ms(1000);}
}

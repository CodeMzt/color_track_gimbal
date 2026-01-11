//
// Created by Mozart on 2025/11/21.
//

#ifndef INVERTEDPENDULUM_PID_H
#define INVERTEDPENDULUM_PID_H

#include <stdint.h>

typedef struct {
    /**pid基本参数**/
    float Kp;
    float Ki;
    float Kd;

    /**pid控制参数**/
    float Target;
    float Current;
    float Last;
    float Out;

    /**pid过程参数**/
    float Err_Integral;
    float Err_Last;
    float Diff_Last;

    /**pid优化系数**/

    /**功能标志位 0000 0000 从低到高：增量式 输入死区 微分先行 积分分离 变速积分 输出偏移 需要则对应标志位置1 **/
    uint8_t Flags;
    //输入死区
    float InputDeadZone_Threshold;
    //低通滤波器
    float Diff_Filter ;
    //积分分离 or 变速积分
    float IntegralSeparation_Threshold;//积分分离阈值
    float VariableIntegral;//变速积分系数
    //输出偏移
    float OutputOffset_Threshold;
    float OutputOffset;

    /**限幅参数**/
    float Out_Max;
    float Out_Min;
    float Err_Integral_Out_Max;
    float Err_Integral_Out_Min;
}PID_T;

void PID_Update(PID_T* p);

#endif //INVERTEDPENDULUM_PID_H
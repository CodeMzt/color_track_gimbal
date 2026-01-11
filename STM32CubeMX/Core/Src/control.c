//
// Created by Mozart on 2026/1/11.
//

#include "../Inc/control.h"

#include "bus_servo.h"
#include "pid.h"

PID_T pid_x_s={
    .Kp = 0.1,
    .Kd = 1,
    .Ki = 0.00,
    .Out_Max = 50,
    .Out_Min = -50,
    .Err_Integral_Out_Max = 50,
    .Err_Integral_Out_Min = -50,
    .Target = 0
}, // 云台001，控制x方向
pid_y_s={
    .Kp = 0.1,
    .Kd = 1,
    .Ki = 0.00,
    .Out_Max = 50,
    .Out_Min = -50,
    .Err_Integral_Out_Max = 50,
    .Err_Integral_Out_Min = -50,
    .Target = 0
};// 云台000，控制y方向

int pwm_x=1500, pwm_y=1500;

int control(UART_HandleTypeDef *huart) {
    if (!huart) return -1;
    char cmd[16];
    PID_Update(&pid_x_s);
    PID_Update(&pid_y_s);
    pwm_x -= (int)pid_x_s.Out;
    pwm_y += (int)pid_y_s.Out;
    if (pwm_x < 1000) pwm_x = 1000;
    if (pwm_x > 2000) pwm_x = 2000;
    if (pwm_y < 1000)
        pwm_y = 1000;
    if (pwm_y > 2000)
        pwm_y = 2000;
    if (bus_servo_fmt_move(cmd, sizeof(cmd), 1,pwm_x,10) < 0) return -1;
    if (bus_servo_send(huart, cmd, 500) != HAL_OK) return -1;
    if (bus_servo_fmt_move(cmd, sizeof(cmd), 0,pwm_y,10) < 0) return -1;
    if (bus_servo_send(huart, cmd, 500) != HAL_OK) return -1;
    return 0;
}

void control_set_target(float x,float y) {
    pid_x_s.Target = x;
    pid_y_s.Target = y;
}

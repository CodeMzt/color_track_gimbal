#ifndef GIMBAL_BUS_SERVO_H
#define GIMBAL_BUS_SERVO_H

#include <stddef.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"

/* 协议常量 */
#define ZSERVO_ID_MIN            0
#define ZSERVO_ID_MAX            254   /* 255 为广播 */
#define ZSERVO_PWM_MIN           500
#define ZSERVO_PWM_MAX           2500
#define ZSERVO_TIME_MAX          9999

/* 固定指令（不含参数），默认目标ID为当前上下文或由调用方在字符串中替换 */
#define CMD_STOP                 "#000PDST!"
#define CMD_PAUSE                "#000PDPT!"
#define CMD_CONTINUE             "#000PDCT!"
#define CMD_BOOT_SAVE_CURRENT    "#000PCSD!"
#define CMD_BOOT_RELEASE_TORQUE  "#000PCSM!"
#define CMD_BOOT_RESTORE_DEFAULT "#000PCSR!"
#define CMD_RELEASE_TORQUE_SOFT  "#000PULK!"
#define CMD_RESTORE_TORQUE       "#000PULR!"
#define CMD_RELEASE_NO_RESIST    "#000PULM!"
#define CMD_FACTORY_RESET_HALF   "#000PCLEO!"
#define CMD_FACTORY_RESET_FULL   "#000PCLE!"
#define CMD_RGB_ON               "#000PLN!"
#define CMD_RGB_OFF              "#000PLF!"

/* 固定读取指令（不含参数） */
#define CMD_PID_READ             "#000PID!"
#define CMD_VER_READ             "#000PVER!"
#define CMD_READ_MODE            "#000PMOD!"
#define CMD_READ_ANGLE           "#000PRAD!"
#define CMD_READ_VOLT_TEMP       "#000PRTV!"
#define CMD_READ_PROTECT         "#000PSTB!"

/* 对外 API：格式化（带目标ID）与收发解析 */
#ifdef __cplusplus
extern "C" {
#endif

/* 发送基础封装 */
HAL_StatusTypeDef bus_servo_send(UART_HandleTypeDef *huart, const char *cmd, uint32_t timeout_ms);

/* 读取到'!'为止（阻塞），返回接收长度，失败返回 -1 */
int bus_servo_recv_until_bang(UART_HandleTypeDef *huart, char *rx_buf, size_t rx_size, uint32_t timeout_ms);

/* 一次发送+读取（常用于查询类指令），返回接收长度，失败返回 -1 */
int bus_servo_send_and_read(UART_HandleTypeDef *huart,
                            const char *cmd,
                            char *rx_buf, size_t rx_size,
                            uint32_t tx_timeout_ms,
                            uint32_t rx_timeout_ms);

/* 带参数的指令格式化函数（返回写入长度，错误返回 -1）。所有函数都会按三位 ID、四位数值进行零填充。 */
int bus_servo_fmt_set_id(char *buf, size_t size, uint8_t from_id, uint8_t to_id);
int bus_servo_fmt_move(char *buf, size_t size, uint8_t id, uint16_t pwm_500_2500, uint16_t time_ms_0_9999);
int bus_servo_fmt_read_id(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_read_ver(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_read_mode(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_read_angle(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_read_volt_temp(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_read_protect(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_pause(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_continue(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_stop(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_release_soft(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_restore_torque(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_set_mode(char *buf, size_t size, uint8_t id, uint8_t mode_1_8);
int bus_servo_fmt_set_baud(char *buf, size_t size, uint8_t id, uint8_t baud_code_0_7);
int bus_servo_fmt_set_sck_base(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_set_boot_pos(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_set_boot_release(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_set_boot_restore(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_set_min(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_set_max(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_set_protect(char *buf, size_t size, uint8_t id, uint8_t value_25_80);
int bus_servo_fmt_set_pi(char *buf, size_t size, uint8_t id, uint16_t kp_0_999, uint16_t ki_0_999);
int bus_servo_fmt_factory_half(char *buf, size_t size, uint8_t id);
int bus_servo_fmt_factory_full(char *buf, size_t size, uint8_t id);

/* 组合批量帧：在整条指令前后加 {}，内含多个单帧，如 {G0000#000P1602T1000!#001P2500T0000!}。
   传入 cmds 为已格式化的小帧数组（每帧以 '!' 结束），本函数将组合并加上花括号。返回组合长度或 -1。*/
int bus_servo_combine_frames(char *out, size_t out_size, const char **cmds, size_t cmd_count, const char *group_prefix);

/* 解析封装（成功返回 0，失败返回 -1）。注意：各设备返回格式以文档为准，解析尽量稳健。 */
int bus_servo_parse_id_response(const char *rx, uint8_t expected_id, uint8_t *out_id);
int bus_servo_parse_ver_response(const char *rx, uint8_t expected_id, char *ver_buf, size_t ver_buf_size);
int bus_servo_parse_mode_response(const char *rx, uint8_t expected_id, uint8_t *out_mode);
int bus_servo_parse_angle_response(const char *rx, uint8_t expected_id, int16_t *out_pwm);
int bus_servo_parse_volt_temp_response(const char *rx, uint8_t expected_id, uint16_t *out_mv, int16_t *out_temp_c);
int bus_servo_parse_ok_response(const char *rx);
int bus_servo_parse_protect_response(const char *rx, uint8_t expected_id, uint8_t *out_value);

/* 高层封装：读取角度（单位度 0..270），内部发送读取角度指令并解析 PWM。返回 0 成功，-1 失败。*/
int bus_servo_get_angle_deg(UART_HandleTypeDef *huart, uint8_t id,
                            uint32_t tx_timeout_ms, uint32_t rx_timeout_ms,
                            int16_t *out_deg);

#ifdef __cplusplus
}
#endif

#endif /* GIMBAL_BUS_SERVO_H */


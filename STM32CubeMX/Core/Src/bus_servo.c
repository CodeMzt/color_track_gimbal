#include "bus_servo.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* 内部工具：安全写入格式化字符串 */
static int write_fmt(char *buf, size_t size, const char *fmt, ...) {
    if (!buf || size == 0) return -1;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    if (n < 0 || (size_t)n >= size) return -1; /* 溢出或错误 */
    return n;
}

/* 跳过响应字符串前导的 CR/LF/空格/制表符 */
static const char* skip_leading_ws(const char *rx) {
    if (!rx) return rx;
    while (*rx == '\r' || *rx == '\n' || *rx == ' ' || *rx == '\t') rx++;
    return rx;
}

HAL_StatusTypeDef bus_servo_send(UART_HandleTypeDef *huart, const char *cmd, uint32_t timeout_ms) {
    if (!huart || !cmd) return HAL_ERROR;
    return HAL_UART_Transmit(huart, (uint8_t*)cmd, (uint16_t)strlen(cmd), timeout_ms);
}

int bus_servo_recv_until_bang(UART_HandleTypeDef *huart, char *rx_buf, size_t rx_size, uint32_t timeout_ms) {
    if (!huart || !rx_buf || rx_size == 0) return -1;
    size_t idx = 0;
    uint8_t ch;
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (HAL_UART_Receive(huart, &ch, 1, 1) == HAL_OK) {
            if (idx + 1 < rx_size) rx_buf[idx++] = (char)ch;
            if (ch == '!') break;
        }
    }
    if (idx == 0) return -1;
    if (idx < rx_size) rx_buf[idx] = '\0';
    return (int)idx;
}

int bus_servo_send_and_read(UART_HandleTypeDef *huart,
                            const char *cmd,
                            char *rx_buf, size_t rx_size,
                            uint32_t tx_timeout_ms,
                            uint32_t rx_timeout_ms) {
    if (!huart || !cmd || !rx_buf || rx_size == 0) return -1;
    if (bus_servo_send(huart, cmd, tx_timeout_ms) != HAL_OK) return -1;
    return bus_servo_recv_until_bang(huart, rx_buf, rx_size, rx_timeout_ms);
}

/* ================= 格式化 ================= */
int bus_servo_fmt_set_id(char *buf, size_t size, uint8_t from_id, uint8_t to_id) {
    if (!buf || size < 12) return -1;
    if (from_id > 255 || to_id > 255) return -1;
    return write_fmt(buf, size, "#%03uPID%03u!", from_id, to_id);
}

int bus_servo_fmt_move(char *buf, size_t size, uint8_t id, uint16_t pwm_500_2500, uint16_t time_ms_0_9999) {
    if (id > 255) return -1;
    if (pwm_500_2500 < ZSERVO_PWM_MIN || pwm_500_2500 > ZSERVO_PWM_MAX) return -1;
    if (time_ms_0_9999 > ZSERVO_TIME_MAX) return -1;
    return write_fmt(buf, size, "#%03uP%04uT%04u!", id, pwm_500_2500, time_ms_0_9999);
}

int bus_servo_fmt_read_id(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPID!", id); }
int bus_servo_fmt_read_ver(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPVER!", id); }
int bus_servo_fmt_read_mode(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPMOD!", id); }
int bus_servo_fmt_read_angle(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPRAD!", id); }
int bus_servo_fmt_read_volt_temp(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPRTV!", id); }
int bus_servo_fmt_read_protect(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPSTB!", id); }

int bus_servo_fmt_pause(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPDPT!", id); }
int bus_servo_fmt_continue(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPDCT!", id); }
int bus_servo_fmt_stop(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPDST!", id); }
int bus_servo_fmt_release_soft(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPULK!", id); }
int bus_servo_fmt_restore_torque(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPULR!", id); }

int bus_servo_fmt_set_mode(char *buf, size_t size, uint8_t id, uint8_t mode_1_8) {
    if (mode_1_8 < 1 || mode_1_8 > 8) return -1;
    return write_fmt(buf, size, "#%03uPMOD%u!", id, mode_1_8);
}

int bus_servo_fmt_set_baud(char *buf, size_t size, uint8_t id, uint8_t baud_code_0_7) {
    if (baud_code_0_7 > 7) return -1; /* 文档定义 0..7 */
    return write_fmt(buf, size, "#%03uPBD%u!", id, baud_code_0_7);
}

int bus_servo_fmt_set_sck_base(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPSCK!", id); }
int bus_servo_fmt_set_boot_pos(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPCSD!", id); }
int bus_servo_fmt_set_boot_release(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPCSM!", id); }
int bus_servo_fmt_set_boot_restore(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPCSR!", id); }
int bus_servo_fmt_set_min(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPSMI!", id); }
int bus_servo_fmt_set_max(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPSMX!", id); }

int bus_servo_fmt_set_protect(char *buf, size_t size, uint8_t id, uint8_t value_25_80) {
    if (value_25_80 < 25 || value_25_80 > 80) return -1;
    return write_fmt(buf, size, "#%03uPSTB=%02u!", id, value_25_80);
}

int bus_servo_fmt_set_pi(char *buf, size_t size, uint8_t id, uint16_t kp_0_999, uint16_t ki_0_999) {
    if (kp_0_999 > 999 || ki_0_999 > 999) return -1;
    return write_fmt(buf, size, "#%03uPP%03uI%03u!", id, kp_0_999, ki_0_999);
}

int bus_servo_fmt_factory_half(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPCLEO!", id); }
int bus_servo_fmt_factory_full(char *buf, size_t size, uint8_t id) { return write_fmt(buf, size, "#%03uPCLE!", id); }

int bus_servo_combine_frames(char *out, size_t out_size, const char **cmds, size_t cmd_count, const char *group_prefix) {
    if (!out || out_size < 4 || (!cmds && cmd_count > 0)) return -1;
    size_t pos = 0;
    int n;
    /* 开始 '{' */
    if (pos + 1 >= out_size) return -1; out[pos++] = '{';
    /* 可选组前缀，如 "G0000" */
    if (group_prefix && group_prefix[0]) {
        n = write_fmt(out + pos, out_size - pos, "%s", group_prefix);
        if (n < 0) return -1; pos += (size_t)n;
    }
    for (size_t i = 0; i < cmd_count; ++i) {
        const char *c = cmds[i]; if (!c) continue; /* 忽略空指针 */
        n = write_fmt(out + pos, out_size - pos, "%s", c);
        if (n < 0) return -1; pos += (size_t)n;
    }
    /* 结束 '}' */
    if (pos + 2 > out_size) return -1; /* '}' + '\0' */
    out[pos++] = '}';
    out[pos] = '\0';
    return (int)pos;
}

/* ================ 解析 ================ */
static int match_prefix_id(const char *rx, uint8_t expected_id) {
    if (!rx) return 0;
    rx = skip_leading_ws(rx);
    size_t len = strlen(rx);
    if (len < 4) return 0;
    char idstr[4]; /* 3位ID */
    snprintf(idstr, sizeof(idstr), "%03u", expected_id);
    return (rx[0] == '#' && rx[1] == idstr[0] && rx[2] == idstr[1] && rx[3] == idstr[2]);
}

int bus_servo_parse_ok_response(const char *rx) {
    if (!rx) return -1;
    rx = skip_leading_ws(rx);
    return (strstr(rx, "#OK!") != NULL) ? 0 : -1;
}

int bus_servo_parse_id_response(const char *rx, uint8_t expected_id, uint8_t *out_id) {
    /* 文档：#000PID! 查询若是该ID返回 #000P!；否则无返回。
       这里放宽处理：若返回以 "#000P" 开头并以 '!' 结束，视为成功，输出 expected_id。*/
    if (!rx || !out_id) return -1;
    rx = skip_leading_ws(rx);
    if (!match_prefix_id(rx, expected_id)) return -1;
    if (strstr(rx, "#") == rx && strstr(rx, "P") == rx + 4) { *out_id = expected_id; return 0; }
    return -1;
}
// 用于解析版本响应：适配新格式，如 "#001@ ZL-ZServo_AD_CBM V2.1.16STG!"
// 解析策略：
// 1) 校验前缀 "#<id>"；2) 查找最后一个 'V'（或第一个 'V'），取其后的内容直到 '!'
// 3) 将该子串拷贝到 ver_buf，作为版本号（不包含 'V' 和 '!')
int bus_servo_parse_ver_response(const char *rx, uint8_t expected_id, char *ver_buf, size_t ver_buf_size) {
    if (!rx || !ver_buf || ver_buf_size == 0) return -1;
    rx = skip_leading_ws(rx);
    if (!match_prefix_id(rx, expected_id)) return -1;
    const char *bang = strrchr(rx, '!');
    if (!bang) return -1;
    // 找到 'V'，优先使用最后一个 'V'，以防厂商名中也含有 'V'
    const char *p = NULL;
    for (const char *q = rx; *q; ++q) {
        if (*q == 'V') p = q; // 记录最后一次出现的位置
    }
    if (!p || p >= bang) return -1; // 没有 'V' 或 'V' 在 '!' 之后无效
    p += 1; // 跳过 'V'
    // 拷贝 [p, bang) 的内容为版本号
    size_t i = 0;
    while (p < bang && i + 1 < ver_buf_size) {
        ver_buf[i++] = *p++;
    }
    ver_buf[i] = '\0';
    return (i > 0) ? 0 : -1;
}

int bus_servo_parse_mode_response(const char *rx, uint8_t expected_id, uint8_t *out_mode) {
    if (!rx || !out_mode) return -1;
    rx = skip_leading_ws(rx);
    if (!match_prefix_id(rx, expected_id)) return -1;
    const char *p = strstr(rx, "PMOD");
    if (!p) {
        p = strstr(rx, "MOD"); // 适配无'P'的返回: "#001MOD1!"
    }
    if (!p) return -1;
    p += (strncmp(p, "PMOD", 4) == 0) ? 4 : 3;
    if (*p >= '1' && *p <= '8') { *out_mode = (uint8_t)(*p - '0'); return 0; }
    return -1;
}

int bus_servo_parse_angle_response(const char *rx, uint8_t expected_id, int16_t *out_pwm) {
    if (!rx || !out_pwm) return -1;
    rx = skip_leading_ws(rx);
    if (!match_prefix_id(rx, expected_id)) return -1;
    const char *p = strstr(rx, "P"); /* 例：#000P1500! */
    if (!p) return -1;
    p += 1;
    int v = 0;
    while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); p++; }
    if (v < ZSERVO_PWM_MIN || v > ZSERVO_PWM_MAX) return -1;
    *out_pwm = (int16_t)v;
    return 0;
}

int bus_servo_parse_volt_temp_response(const char *rx, uint8_t expected_id, uint16_t *out_mv, int16_t *out_temp_c) {
    if (!rx || !out_mv || !out_temp_c) return -1;
    rx = skip_leading_ws(rx);
    if (!match_prefix_id(rx, expected_id)) return -1;
    /* 返回可能形如：#000PRTV12000,35! 或 #000PRTV12000 35! */
    const char *p = strstr(rx, "PRTV");
    if (!p) return -1;
    p += 4;
    int mv = 0; while (*p >= '0' && *p <= '9') { mv = mv * 10 + (*p - '0'); p++; }
    while (*p && *p != '-' && (*p < '0' || *p > '9')) { if (*p == '!') break; p++; }
    int temp = 0; int sign = 1;
    if (*p == '-') { sign = -1; p++; }
    while (*p >= '0' && *p <= '9') { temp = temp * 10 + (*p - '0'); p++; }
    *out_mv = (uint16_t)mv;
    *out_temp_c = (int16_t)(temp * sign);
    return 0;
}

int bus_servo_parse_protect_response(const char *rx, uint8_t expected_id, uint8_t *out_value) {
    if (!rx || !out_value) return -1;
    rx = skip_leading_ws(rx);
    if (!match_prefix_id(rx, expected_id)) return -1;
    const char *p = strstr(rx, "PSTB");
    if (!p) return -1;
    p += 4;
    if (*p == '=') p++;
    int v = 0; while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); p++; }
    if (v < 0 || v > 255) return -1;
    *out_value = (uint8_t)v;
    return 0;
}

int bus_servo_get_angle_deg(UART_HandleTypeDef *huart, uint8_t id,
                            uint32_t tx_timeout_ms, uint32_t rx_timeout_ms,
                            int16_t *out_deg) {
    if (!huart || !out_deg) return -1;
    char cmd[16];
    if (bus_servo_fmt_read_angle(cmd, sizeof(cmd), id) < 0) return -1;
    char rx[64];
    int n = bus_servo_send_and_read(huart, cmd, rx, sizeof(rx), tx_timeout_ms, rx_timeout_ms);
    if (n <= 0) return -1;
    int16_t pwm = 0;
    if (bus_servo_parse_angle_response(rx, id, &pwm) != 0) return -1;
    /* 将 500..2500 映射到 0..270 度。线性：deg = (pwm-500) * 270 / 2000，四舍五入 */
    int32_t num = (int32_t)(pwm - ZSERVO_PWM_MIN) * 270 + 1000; /* +1000 近似实现 0.5 舍入 */
    int16_t deg = (int16_t)(num / 2000);
    if (deg < 0) deg = 0; if (deg > 270) deg = 270;
    *out_deg = deg;
    return 0;
}


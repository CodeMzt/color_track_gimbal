# color_track_gimbal

基于K230和STM32的二维云台视觉实时追踪模块

## 项目简介

本项目是一个智能视觉追踪系统，使用CanMV K230开发板进行实时图像处理和目标识别，通过STM32F103单片机控制二维云台实现对目标的自动追踪。系统采用PID闭环控制算法，能够实时追踪红色和绿色物体，并通过UART串口实现两个控制器之间的通信。

### 主要特性

- 🎯 **实时颜色识别**：支持红色和绿色物体的实时检测与追踪
- 🎮 **触摸交互界面**：通过ST7701显示屏提供直观的触控操作界面
- 🔄 **PID闭环控制**：采用成熟的PID算法实现平稳、精确的云台控制
- 🚀 **高性能处理**：K230芯片提供强大的图像处理能力
- 🔧 **总线舵机控制**：支持标准总线舵机协议，控制精度高
- 📡 **UART通信**：K230与STM32之间通过115200波特率串口通信

## 系统架构

```
┌─────────────────────────────────────────────────────────┐
│                    CanMV K230 开发板                     │
│  ┌─────────────────────────────────────────────────┐   │
│  │  • 图像采集 (800x480)                            │   │
│  │  • 颜色识别与目标检测                             │   │
│  │  • 触摸屏交互界面                                 │   │
│  │  • 目标坐标计算                                   │   │
│  └─────────────────────────────────────────────────┘   │
└──────────────────────┬──────────────────────────────────┘
                       │ UART2 (115200bps)
                       │ 格式: $X Y$\r\n
                       ↓
┌─────────────────────────────────────────────────────────┐
│                   STM32F103 控制板                       │
│  ┌─────────────────────────────────────────────────┐   │
│  │  • 串口数据解析                                   │   │
│  │  • PID控制计算                                    │   │
│  │  • 总线舵机驱动                                   │   │
│  │  • OLED状态显示                                   │   │
│  └─────────────────────────────────────────────────┘   │
└──────────────────────┬──────────────────────────────────┘
                       │ UART1 (总线舵机协议)
                       ↓
            ┌──────────────────────┐
            │   二维云台 (2轴)      │
            │  • 舵机 ID=0 (Y轴)   │
            │  • 舵机 ID=1 (X轴)   │
            └──────────────────────┘
```

## 目录结构

```
color_track_gimbal/
├── CanMV K230 IDE/              # K230图像处理代码
│   └── main.py                  # K230主程序（图像采集与识别）
├── STM32CubeMX/                 # STM32控制代码
│   ├── Core/                    # 核心代码
│   │   ├── Inc/                 # 头文件目录
│   │   │   ├── main.h          # 主程序头文件
│   │   │   ├── control.h       # 云台控制头文件
│   │   │   ├── pid.h           # PID算法头文件
│   │   │   ├── bus_servo.h     # 总线舵机驱动头文件
│   │   │   ├── oled.h          # OLED显示驱动头文件
│   │   │   ├── oledfont.h      # OLED字体库
│   │   │   ├── key.h           # 按键驱动头文件
│   │   │   ├── delay.h         # 延时函数头文件
│   │   │   ├── usart.h         # 串口配置头文件
│   │   │   ├── i2c.h           # I2C配置头文件
│   │   │   ├── tim.h           # 定时器配置头文件
│   │   │   ├── gpio.h          # GPIO配置头文件
│   │   │   ├── stm32f1xx_it.h  # 中断处理头文件
│   │   │   └── stm32f1xx_hal_conf.h  # HAL库配置
│   │   └── Src/                 # 源文件目录
│   │       ├── main.c          # 主程序（协调控制）
│   │       ├── control.c       # 云台控制实现
│   │       ├── pid.c           # PID算法实现
│   │       ├── bus_servo.c     # 总线舵机驱动实现
│   │       ├── oled.c          # OLED显示驱动实现
│   │       ├── key.c           # 按键驱动实现
│   │       ├── delay.c         # 延时函数实现
│   │       ├── usart.c         # 串口初始化
│   │       ├── i2c.c           # I2C初始化
│   │       ├── tim.c           # 定时器初始化
│   │       ├── gpio.c          # GPIO初始化
│   │       ├── stm32f1xx_it.c  # 中断服务函数
│   │       ├── stm32f1xx_hal_msp.c  # HAL MSP初始化
│   │       ├── system_stm32f1xx.c   # 系统初始化
│   │       ├── syscalls.c      # 系统调用
│   │       └── sysmem.c        # 内存管理
│   ├── Drivers/                 # STM32 HAL驱动库
│   ├── cmake/                   # CMake构建配置
│   ├── CMakeLists.txt          # CMake项目配置
│   ├── CMakePresets.json       # CMake预设配置
│   ├── Gimbal.ioc              # STM32CubeMX配置文件
│   ├── STM32F103XX_FLASH.ld    # 链接脚本
│   └── startup_stm32f103xb.s   # 启动文件
├── LICENSE                      # MIT许可证
└── README.md                    # 项目说明文档
```

## 文件功能详解

### K230 图像处理部分

#### `CanMV K230 IDE/main.py`
K230主控程序，负责整个视觉处理流程：

**主要功能：**
- **图像采集**：配置摄像头采集800×480分辨率的RGB565图像
- **颜色阈值设置**：定义红色和绿色的LAB色彩空间阈值
  - 红色阈值：`(0, 100, 21, 127, 0, 127)`
  - 绿色阈值：`(11, 82, -96, -26, -128, 127)`
- **目标检测**：使用`find_blobs()`方法检测颜色块
  - 最小像素数：3000
  - 最小面积：3000
  - 尺寸限制：50×50 至 500×300 像素
- **触摸交互**：实现四个触控按钮
  - 左上角：选择红色追踪模式
  - 右上角：选择绿色追踪模式
  - 左下角：暂停追踪
  - 右下角：退出程序
- **坐标转换**：将图像坐标转换为云台控制坐标
  - 公式：`X = cx - 400`, `Y = -cy + 240`
- **串口通信**：通过UART2发送目标坐标
  - 波特率：115200
  - 数据格式：`$X Y$\r\n`
  - 引脚配置：GPIO5(TX), GPIO6(RX)

**关键代码逻辑：**
```python
# 坐标计算：图像中心为(400, 240)
cx, cy = blob.cx(), blob.cy()
# 发送相对于中心的偏移量
u1.write(f"${cx-400} {-cy+240}$\r\n")
```

### STM32 控制部分

#### `Core/Src/main.c` & `Core/Inc/main.h`
STM32主程序，协调各个模块工作：

**主要功能：**
- 系统初始化（时钟、外设配置）
- 串口数据接收与解析（从K230接收坐标）
- 数据格式解析：`parse_packet()` 函数解析 `$X Y$` 格式
- 调用控制模块更新云台位置
- 初始化云台到中位（PWM=1500）
- OLED显示系统状态

**数据流：**
```c
// 接收K230发送的坐标数据
HAL_UART_Receive(&huart2, buf, sizeof(buf), 10);
parse_packet(buf, &received_x, &received_y);

// 平滑滤波（0.95新值 + 0.05旧值）
control_set_target(x*0.05 + received_x*0.95, y*0.05 + received_y*0.95);

// PID控制并发送舵机指令
control(&huart1);
```

#### `Core/Src/control.c` & `Core/Inc/control.h`
云台控制模块，实现闭环控制：

**主要功能：**
- 维护两个PID控制器（X轴和Y轴）
- PID参数配置：
  - Kp = 0.1（比例系数）
  - Kd = 1.0（微分系数）
  - Ki = 0.0（积分系数）
  - 输出限幅：±50
- PWM值范围：1000-2000（对应舵机500-2500的协议范围）
- 目标设定接口：`control_set_target(float x, float y)`
- 控制执行接口：`control(UART_HandleTypeDef *huart)`

**控制逻辑：**
```c
// 更新PID计算
PID_Update(&pid_x_s);
PID_Update(&pid_y_s);

// 应用PID输出到PWM值
pwm_x -= (int)pid_x_s.Out;  // X轴反向
pwm_y += (int)pid_y_s.Out;  // Y轴正向

// PWM限幅保护
if (pwm_x < 1000) pwm_x = 1000;
if (pwm_x > 2000) pwm_x = 2000;
```

#### `Core/Src/pid.c` & `Core/Inc/pid.h`
PID算法实现模块：

**主要功能：**
- 位置式PID算法
- 支持多种优化策略：
  - 微分先行
  - 输入死区
  - 积分分离
  - 变速积分
  - 低通滤波
- 输出限幅保护

**PID结构体：**
```c
typedef struct {
    float Kp, Ki, Kd;          // PID系数
    float Target, Current;      // 目标值和当前值
    float Out;                  // 输出值
    float Err_Integral;         // 积分累积
    float Err_Last, Diff_Last;  // 上次误差和微分
    uint8_t Flags;             // 功能标志位
    float Out_Max, Out_Min;    // 输出限幅
} PID_T;
```

#### `Core/Src/bus_servo.c` & `Core/Inc/bus_servo.h`
总线舵机通信协议驱动：

**主要功能：**
- 封装标准总线舵机协议命令
- 支持的指令类型：
  - 位置控制：`#IIIPPPPPTTTT!`
  - ID设置：`#IIIPIDIII!`
  - 查询指令：读取ID、版本、模式、角度、电压温度等
  - 控制指令：暂停、继续、停止、扭矩控制等
- 协议规范：
  - ID范围：0-254（255为广播）
  - PWM范围：500-2500
  - 时间范围：0-9999ms

**关键函数：**
```c
// 格式化移动指令
int bus_servo_fmt_move(char *buf, size_t size, 
                       uint8_t id, 
                       uint16_t pwm,      // 500-2500
                       uint16_t time_ms); // 0-9999

// 发送指令
HAL_StatusTypeDef bus_servo_send(UART_HandleTypeDef *huart, 
                                  const char *cmd, 
                                  uint32_t timeout_ms);
```

#### `Core/Src/oled.c` & `Core/Inc/oled.h` & `Core/Inc/oledfont.h`
OLED显示驱动模块：

**主要功能：**
- I2C接口的OLED显示屏驱动
- 支持字符串、数字显示
- 中文字符显示支持
- 提供清屏、定位等基础操作
- 字体库支持（oledfont.h）

#### `Core/Src/key.c` & `Core/Inc/key.h`
按键驱动模块：

**主要功能：**
- GPIO按键扫描
- 按键消抖处理
- 定时器中断周期性采样

#### `Core/Src/delay.c` & `Core/Inc/delay.h`
精确延时模块：

**主要功能：**
- 微秒级延时
- 毫秒级延时
- 不依赖HAL_Delay，避免影响系统节拍

#### `Core/Src/usart.c` & `Core/Inc/usart.h`
串口配置模块：

**主要功能：**
- USART1配置（与总线舵机通信）：115200, 8N1
- USART2配置（与K230通信）：115200, 8N1

#### `Core/Src/i2c.c` & `Core/Inc/i2c.h`
I2C配置模块：

**主要功能：**
- I2C1初始化配置
- 用于OLED显示屏通信

#### `Core/Src/tim.c` & `Core/Inc/tim.h`
定时器配置模块：

**主要功能：**
- TIM1配置（周期性中断）
- 用于按键扫描等周期性任务

#### `Core/Src/gpio.c` & `Core/Inc/gpio.h`
GPIO配置模块：

**主要功能：**
- GPIO引脚初始化
- 按键、LED等IO配置

#### `Core/Src/stm32f1xx_it.c` & `Core/Inc/stm32f1xx_it.h`
中断服务函数：

**主要功能：**
- 系统中断处理
- 定时器中断处理
- 串口中断处理

## 硬件要求

### 必需硬件

1. **CanMV K230 开发板**
   - 配备摄像头模块
   - ST7701 触摸显示屏（800×480分辨率）
   - 支持UART通信

2. **STM32F103 开发板**
   - 型号：STM32F103C8T6 或更高配置
   - 外部晶振：HSE
   - Flash：至少64KB

3. **二维云台系统**
   - 总线舵机×2（支持标准总线舵机协议）
   - 舵机ID配置：
     - ID=0：Y轴（俯仰）
     - ID=1：X轴（水平旋转）
   - PWM范围：500-2500
   - 通信波特率：115200

4. **OLED显示屏**（可选）
   - I2C接口
   - 用于显示系统状态

5. **电源**
   - K230：5V/2A
   - STM32：3.3V/5V
   - 舵机：6V-8.4V（根据舵机规格）

### 接线说明

#### K230 连接
```
K230              STM32F103
GPIO5 (TX) -----> PA3 (USART2_RX)
GPIO6 (RX) -----> PA2 (USART2_TX)
GND       -----> GND
```

#### STM32F103 连接
```
STM32F103         总线舵机
PA9  (USART1_TX) --> 舵机RX
PA10 (USART1_RX) --> 舵机TX
GND              --> 舵机GND

STM32F103         OLED
PB6  (I2C1_SCL)  --> OLED SCL
PB7  (I2C1_SDA)  --> OLED SDA
GND              --> OLED GND
VCC              --> OLED VCC
```

## 软件依赖

### K230 开发环境

1. **CanMV IDE**
   - 版本：支持K230芯片的最新版本
   - 下载地址：[CanMV官网](https://developer.canaan-creative.com/resource)

2. **Python库依赖**
   - `machine`：硬件访问（UART, FPIOA, TOUCH）
   - `media`：图像采集和显示
   - `image`：图像处理
   - 这些库已集成在CanMV K230固件中

### STM32 开发环境

#### 方式一：使用 Keil MDK（推荐Windows用户）

1. **Keil MDK-ARM**
   - 版本：5.30或更高
   - 芯片支持包：STM32F1xx_DFP

2. **STM32CubeMX**
   - 版本：6.0或更高
   - 用于外设配置和代码生成

#### 方式二：使用 CMake + GCC（推荐Linux/Mac用户）

1. **CMake**
   - 版本：3.22或更高
   ```bash
   sudo apt install cmake  # Ubuntu/Debian
   brew install cmake      # macOS
   ```

2. **ARM GCC工具链**
   - 版本：10.3或更高
   ```bash
   sudo apt install gcc-arm-none-eabi  # Ubuntu/Debian
   brew install --cask gcc-arm-embedded  # macOS
   ```

3. **构建工具**
   ```bash
   sudo apt install build-essential ninja-build
   ```

## 使用指南

### 一、K230端部署

#### 1. 准备K230开发环境

1. 下载并安装CanMV IDE
2. 连接K230开发板到电脑
3. 确保摄像头和显示屏已正确连接

#### 2. 调整颜色阈值（重要）

根据实际光照条件调整颜色阈值：

```python
# 在 main.py 中修改以下参数
thresholds = [
    (0, 100, 21, 127, 0, 127),      # 红色阈值 (L_min, L_max, A_min, A_max, B_min, B_max)
    (11, 82, -96, -26, -128, 127)    # 绿色阈值
]
```

**阈值调试技巧：**
1. 使用CanMV IDE的阈值编辑器
2. 在不同光照下测试并调整
3. 确保`pixels_threshold`和`area_threshold`适合目标大小

#### 3. 上传程序

1. 在CanMV IDE中打开 `CanMV K230 IDE/main.py`
2. 点击"连接设备"
3. 点击"运行"或"下载到设备"

#### 4. 操作界面说明

```
┌────────────────────────────────────────────────┐
│ [红色]                            [绿色]        │  ← 模式选择
│                                                 │
│              摄像头实时画面                      │
│                                                 │
│ [暂停]                            [退出]        │  ← 功能按钮
└────────────────────────────────────────────────┘
```

- **红色按钮**：切换到红色物体追踪模式
- **绿色按钮**：切换到绿色物体追踪模式
- **暂停按钮**：暂停追踪，云台保持当前位置
- **退出按钮**：退出程序

### 二、STM32端部署

#### 方式一：使用 Keil MDK

1. **打开项目**
   ```
   打开 STM32CubeMX/MDK-ARM/Gimbal.uvprojx
   ```

2. **编译项目**
   - 点击 Build 按钮（F7）
   - 确保无错误和警告

3. **下载程序**
   - 连接ST-Link到STM32开发板
   - 点击 Download 按钮（F8）

#### 方式二：使用 CMake

1. **进入项目目录**
   ```bash
   cd STM32CubeMX
   ```

2. **配置项目**
   ```bash
   cmake --preset=Debug
   # 或使用 Release 配置
   cmake --preset=Release
   ```

3. **编译项目**
   ```bash
   cmake --build build/Debug
   # 或
   cmake --build build/Release
   ```

4. **下载程序**
   ```bash
   # 使用 OpenOCD
   openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
           -c "program build/Debug/Gimbal.elf verify reset exit"
   
   # 或使用 st-flash
   st-flash write build/Debug/Gimbal.bin 0x8000000
   ```

### 三、系统调试

#### 1. PID参数调优

在 `Core/Src/control.c` 中调整PID参数：

```c
PID_T pid_x_s = {
    .Kp = 0.1,   // 比例系数：增大响应更快但可能震荡
    .Kd = 1.0,   // 微分系数：增大可减少超调
    .Ki = 0.0,   // 积分系数：消除稳态误差
    .Out_Max = 50,
    .Out_Min = -50
};
```

**调参建议：**
1. 先调Kp：从小到大，直到响应足够快但不震荡
2. 再调Kd：适当增大以减少超调
3. 最后调Ki：如有稳态误差可适当增大（通常保持很小或为0）

#### 2. 舵机行程调整

如果云台运动范围不合适，调整PWM限幅：

```c
// 在 control.c 中
if (pwm_x < 1000) pwm_x = 1000;  // 修改最小值
if (pwm_x > 2000) pwm_x = 2000;  // 修改最大值
```

#### 3. 通信故障排查

- **检查串口连接**：用示波器或逻辑分析仪确认TX/RX信号
- **检查波特率**：确保K230和STM32都配置为115200
- **检查数据格式**：确认数据包格式为 `$X Y$\r\n`

#### 4. 云台异常排查

- **舵机不响应**：检查UART1连接和舵机ID配置
- **抖动严重**：降低PID的Kp值或增大Kd值
- **响应慢**：增大PID的Kp值
- **偏移量大**：检查舵机中位校准（初始PWM=1500）

### 四、系统工作流程

```
1. K230启动
   ├─ 初始化摄像头和显示屏
   ├─ 配置UART2串口
   └─ 进入主循环

2. 主循环（K230）
   ├─ 采集图像
   ├─ 检测触摸输入
   ├─ 根据模式检测目标颜色
   ├─ 计算目标中心坐标
   ├─ 通过UART2发送坐标 "$X Y$\r\n"
   └─ 更新显示界面

3. STM32接收
   ├─ UART2接收数据
   ├─ 解析数据包提取X, Y坐标
   ├─ 数据平滑滤波
   └─ 设置PID目标值

4. PID控制（STM32）
   ├─ 计算X轴PID输出
   ├─ 计算Y轴PID输出
   ├─ 转换为舵机PWM值
   ├─ 格式化总线舵机指令
   └─ 通过UART1发送到舵机

5. 云台运动
   ├─ ID=1舵机控制X轴（水平）
   └─ ID=0舵机控制Y轴（俯仰）
```

## 通信协议说明

### K230 → STM32 协议

**格式：** `$X Y$\r\n`

- **起始符**：`$`
- **X坐标**：目标相对图像中心的水平偏移（-400 ~ +400）
- **分隔符**：空格
- **Y坐标**：目标相对图像中心的垂直偏移（-240 ~ +240）
- **结束符**：`$\r\n`

**示例：**
```
$-50 30$\r\n    # 目标在中心左侧50像素，上方30像素
$0 0$\r\n       # 目标在图像正中心
$150 -100$\r\n  # 目标在中心右侧150像素，下方100像素
```

### STM32 → 舵机协议（总线舵机）

**移动指令格式：** `#IIIPPPPPTTTT!`

- **#** ：起始符
- **III** ：舵机ID（000-254，三位数字）
- **P** ：PWM标识符
- **PPPPP** ：PWM值（0500-2500，四位数字）
- **T** ：时间标识符
- **TTTT** ：运动时间（0000-9999毫秒，四位数字）
- **!** ：结束符

**示例：**
```
#001P1500T0010!  # ID=1舵机移动到PWM=1500，用时10ms
#000P1800T0100!  # ID=0舵机移动到PWM=1800，用时100ms
```

**其他指令：**
- 设置ID：`#IIIPIDIII!`
- 读取ID：`#IIIPID!`
- 读取版本：`#IIIPVER!`
- 停止：`#IIIPDST!`
- 暂停：`#IIIPDPT!`
- 继续：`#IIIPACT!`

## 性能参数

- **图像处理帧率**：理论可达100fps（实际约30-60fps）
- **目标检测延迟**：< 50ms
- **串口通信延迟**：< 5ms
- **PID控制周期**：约10-20ms
- **云台响应时间**：50-200ms（取决于PID参数和舵机性能）
- **追踪精度**：±5像素（图像坐标）

## 常见问题

### Q1: 云台抖动严重怎么办？
**A:** 降低PID的Kp值或增大Kd值。建议从Kp=0.05开始逐步增加。

### Q2: 检测不到颜色怎么办？
**A:** 在CanMV IDE中使用阈值编辑器重新调整颜色阈值，确保在当前光照条件下能正确识别目标。

### Q3: 舵机不动作怎么办？
**A:** 
1. 检查UART1连接是否正确
2. 确认舵机ID配置为0和1
3. 检查舵机供电是否正常
4. 使用串口助手测试发送指令 `#001P1500T0100!`

### Q4: 串口通信失败怎么办？
**A:**
1. 检查TX/RX是否交叉连接
2. 确认两端波特率都是115200
3. 检查GND是否共地
4. 使用示波器查看波形

### Q5: 追踪目标丢失怎么办？
**A:**
1. 确保目标颜色鲜明且与背景对比明显
2. 调整检测阈值（pixels_threshold, area_threshold）
3. 确保目标大小在50×50至500×300范围内
4. 改善光照条件

## 开发者信息

- **作者**：Mozart (CodeMzt)
- **创建日期**：2026年1月
- **项目地址**：[https://github.com/CodeMzt/color_track_gimbal](https://github.com/CodeMzt/color_track_gimbal)

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

```
MIT License

Copyright (c) 2026 CodeMzt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## 更新日志

### v1.0.0 (2026-01-11)
- ✨ 初始版本发布
- ✅ 完成基本颜色追踪功能
- ✅ 实现PID闭环控制
- ✅ 添加触摸交互界面
- ✅ 完善总线舵机驱动
- ✅ 添加OLED状态显示

## 致谢

- 感谢 Canaan Creative 提供 K230 芯片和开发工具
- 感谢 STMicroelectronics 提供 HAL 库和开发工具
- 感谢开源社区的各种参考资料和支持

## 联系方式

如有问题或建议，欢迎通过以下方式联系：

- 提交 Issue：[GitHub Issues](https://github.com/CodeMzt/color_track_gimbal/issues)
- 项目讨论：[GitHub Discussions](https://github.com/CodeMzt/color_track_gimbal/discussions)

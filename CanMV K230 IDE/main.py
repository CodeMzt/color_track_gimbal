# color_track - By: Mozart - Sat Jan 10 2026

import time, os, gc, sys, math
from machine import TOUCH
from media.display import *
from media.media import *
from media.sensor import *
from machine import UART
from machine import FPIOA
import image


fpioa = FPIOA()
fpioa.set_function(5, FPIOA.UART2_TXD)  # 启用上拉
fpioa.set_function(6, FPIOA.UART2_RXD)

u1 = UART(
    UART.UART2,           # UART2 通道
    baudrate=115200,      # 波特率 115200
    bits=UART.EIGHTBITS,  # 数据位 8 位
    parity=UART.PARITY_NONE,  # 无校验
    stop=UART.STOPBITS_ONE    # 停止位 1 位
)


thresholds = [(0, 100, 21, 127, 0, 127), # generic_red_thresholds -> index is 0 so code == (1 << 0)
              (11, 82, -96, -26, -128, 127)] # generic_green_thresholds -> index is 1 so code == (1 << 1)

# 配置显示参数
DISPLAY_TYPE = Display.ST7701
DISPLAY_WIDTH = 800
DISPLAY_HEIGHT = 480

flag = 0

sensor=None

try:
    touch = TOUCH(0)  # 触摸设备0

    sensor = Sensor(width = DISPLAY_WIDTH, height = DISPLAY_HEIGHT)
    sensor.reset()
    # 设置通道 0
    sensor.set_vflip(True)
    sensor.set_hmirror(True)
    sensor.set_framesize(width=DISPLAY_WIDTH, height= DISPLAY_HEIGHT, chn = CAM_CHN_ID_0)
    sensor.set_pixformat(Sensor.RGB565, chn=CAM_CHN_ID_0)
#    bind_info = sensor.bind_info()
#    Display.bind_layer(**bind_info, layer=Display.LAYER_VIDEO1)

    Display.init(DISPLAY_TYPE, width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT,fps=100, to_ide=False)
    MediaManager.init()
    sensor.run()
    cx,cy=400,240
    while True:
        cx,cy=400,240
#        time.sleep_ms(10)  # 降低更新频率
        try:
            # 读取触摸点 - 添加异常处理
            points = touch.read()
            # 检查points是否有效且非空
            if points and isinstance(points, (list, tuple)) and len(points) > 0:
                for i, point in enumerate(points):
                    # 检查point是否有x, y, event属性
                    x, y = point.x, point.y
                    event = point.event  # 0:按下, 1:移动, 2:抬起

                    if event == TOUCH.EVENT_UP:  # 抬起
                        if y >= 410 and y <= 470:  # 410+60=470
                            if x >= 10 and x <= 110:  # 10+100=110
                                flag=0
                            elif x >= 690 and x <= 790:  # 690+100=790
                                raise KeyboardInterrupt
                        if y >= 10 and y <= 70:  # 410+60=470
                            if x >= 10 and x <= 110:  # 10+100=110
                                flag=1
                            elif x >= 690 and x <= 790:  # 690+100=790
                                flag=2
            img = sensor.snapshot()
            if flag:
                for blob in img.find_blobs(thresholds, pixels_threshold=3000, area_threshold=3000, merge=True):
                    if flag == 1:
                        if blob.code() == 1 and blob.w()>=50 and blob.h()>=50 and blob.w()<500 and blob.h()<300: # r/g code == (1 << 1) | (1 << 0)
                            cx,cy=blob.cx(),blob.cy()
                            img.draw_rectangle([v for v in blob.rect()])
                    elif flag == 2 :
                        if blob.code() == 2 and blob.w()>=50 and blob.h()>=50 and blob.w()<500 and blob.h()<300: # r/g code == (1 << 1) | (1 << 0)
                            cx,cy=blob.cx(),blob.cy()
                            img.draw_rectangle([v for v in blob.rect()])

            u1.write(f"${cx-400} {-cy+240}$\r\n")  # 发送字符串
#             print((f"${cx-400} {-cy+240}$\r\n"))
            # draw result to screen
            # 交互界面创建
            img.draw_rectangle(10,10,100,60, color = (255,255,255),  fill = True)
            img.draw_rectangle(690,10,100,60, color = (255,255,255),  fill = True)
            img.draw_rectangle(10,410,100,60, color = (255,255,255),  fill = True)
            img.draw_rectangle(690,410,100,60, color = (255,255,255), fill = True)
            img.draw_string_advanced(10,10,50, "红色", color=(0, 0, 0), scale=4)
            img.draw_string_advanced(690,10,50, "绿色", color=(0, 0, 0), scale=4)
            # 左下角按键文字
            img.draw_string_advanced(10,410,50, "暂停", color=(0, 0, 0), scale=4)
            # 右下角按键文字
            img.draw_string_advanced(690,410,50, "退出", color=(0, 0, 0), scale=4)
            Display.show_image(img)
            gc.collect()
        except Exception as e:
            raise e

except KeyboardInterrupt as e:
    print(f"user stop")
except BaseException as e:
    print(f"Exception '{e}'")
finally:
    # ========== 释放资源 ==========
    u1.deinit()
    # sensor stop run
    if isinstance(sensor, Sensor):
        sensor.stop()
    # deinit display
    Display.deinit()
    os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
    time.sleep_ms(100)

    # release media buffer
    MediaManager.deinit()

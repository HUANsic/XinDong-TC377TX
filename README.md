# 芯动计划2025 - TC377TX智能车项目

## 📋 项目概述

本项目是基于英飞凌TC377TX三核微控制器的智能车控制系统，专为芯动计划2025设计。系统采用多核并行处理架构，集成了计算机视觉、运动控制、传感器融合等先进技术，实现智能循迹、避障、通信等功能。

## 🏗️ 系统架构

### 多核处理器架构
- **CPU0**: 主控制核心，负责系统初始化、安全监控和协调
- **CPU1**: 图像处理核心，专门处理摄像头数据和计算机视觉算法
- **CPU2**: 运动控制核心，负责电机控制、传感器读取和通信

### 硬件平台
- **主控芯片**: Infineon TC377TX (TriCore™)
- **图像传感器**: OV7670摄像头 (188×120分辨率)
- **运动系统**: 双轮差速驱动 + 编码器反馈
- **传感器**: MPU6050 IMU、超声波传感器、干簧管
- **通信**: 蓝牙模块、串口通信
- **显示**: OLED显示屏

## 🔧 核心功能模块

### 1. 图像处理系统 (CV模块)
- **实时图像采集**: 188×120分辨率，30fps
- **三缓冲管理**: 避免数据竞争，确保图像完整性
- **智能中线检测**: 
  - 从下往上扫描算法
  - 自适应分割线技术
  - 补线方向判断
  - 偏差计算与输出

### 2. 运动控制系统
- **双轮差速驱动**: 精确的转向控制
- **编码器反馈**: 实时速度测量
- **PID控制算法**: 稳定的运动控制
- **多种运动模式**: 直线、转弯、停车等

### 3. 传感器融合
- **IMU数据**: 姿态角、加速度、角速度
- **超声波测距**: 障碍物检测
- **干簧管**: 位置检测
- **电位器**: 参数调节

### 4. 通信系统
- **蓝牙通信**: 无线数据传输
- **串口通信**: 调试和参数配置
- **多核间通信**: 核心间数据同步

## 📁 项目结构

```
XinDong2025Code/
├── XinDong_TC377TX_Demo_v0_1/          # 主项目目录
│   ├── Cpu0_Main.c                     # CPU0主程序
│   ├── Cpu1_Main.c                     # CPU1主程序  
│   ├── Cpu2_Main.c                     # CPU2主程序
│   ├── XinDongLib/                     # 核心库文件
│   │   ├── Camera.c/h                  # 摄像头驱动
│   │   ├── CV.c/h                      # 计算机视觉算法
│   │   ├── Movements.c/h               # 运动控制
│   │   ├── IMU.c/h                     # 惯性测量单元
│   │   ├── Encoder.c/h                 # 编码器
│   │   ├── Ultrasonic.c/h              # 超声波传感器
│   │   ├── Bluetooth.c/h               # 蓝牙通信
│   │   ├── Display.c/h                 # OLED显示
│   │   ├── Serial.c/h                  # 串口通信
│   │   ├── Time.c/h                    # 时间管理
│   │   ├── IO.c/h                      # 输入输出控制
│   │   ├── Intercore.c/h               # 多核通信
│   │   ├── Interrupts.c/h              # 中断管理
│   │   └── XinDong_Config.h            # 系统配置
│   └── Libraries/                      # 英飞凌底层库
└── xindong_template_tc377tx/           # 项目模板
```

## 🚀 快速开始

### 环境要求
- **开发工具**: Infineon AURIX Development Studio
- **编译器**: Tasking VX Toolset for TriCore
- **硬件**: TC377TX开发板及相关外设

### 编译步骤
1. 打开AURIX Development Studio
2. 导入项目: `File → Import → Existing Projects into Workspace`
3. 选择项目目录: `XinDong_TC377TX_Demo_v0_1`
4. 配置编译选项和链接脚本
5. 编译项目: `Project → Build All`

### 烧录程序
1. 连接调试器 (如J-Link)
2. 配置调试设置
3. 下载程序到目标板
4. 启动调试或运行

## 💡 使用指南

### 图像处理使用
```c
#include "XinDongLib/CV.h"

// 在主循环中调用
CV_Result_t cv_result = CV_ProcessImage();

if (cv_result.valid) {
    int16 error = cv_result.error;      // 获取偏差值
    int16 buxian = g_buxian;            // 获取补线方向
    
    // 根据偏差控制转向
    if (error > 0) {
        // 右转控制
    } else if (error < 0) {
        // 左转控制
    }
}
```

### 运动控制使用
```c
#include "XinDongLib/Movements.h"

// 设置运动参数
Movements_SetSpeed(50);     // 设置速度
Movements_SetDirection(1);  // 设置方向

// 执行运动
Movements_Start();
```

### 传感器读取
```c
#include "XinDongLib/IMU.h"
#include "XinDongLib/Ultrasonic.h"

// 读取IMU数据
float angle = IMU_GetAngle();
float accel = IMU_GetAcceleration();

// 读取超声波距离
uint16 distance = Ultrasonic_GetDistance();
```

## 🔧 配置说明

### 引脚配置
所有硬件引脚定义在 `XinDong_Config.h` 中：
- 摄像头接口: P00.1-2 (SCCB), P02.0-7 (数据), P00.7-9 (控制)
- 电机控制: P21.2-3 (PWM输出)
- 编码器: P33.6-7 (正交编码)
- IMU/OLED: P13.1-2 (I2C)
- 蓝牙: P33.8-10 (UART)
- LED/DIP开关: P20.7-14

### 参数调节
- **PID参数**: 在 `Movements.h` 中配置
- **图像阈值**: 在 `CV.c` 中调整二值化阈值
- **通信参数**: 在 `Bluetooth.c` 和 `Serial.c` 中设置

## 📊 性能指标

- **图像处理**: 188×120@30fps
- **控制周期**: 50ms (20Hz)
- **响应时间**: <10ms
- **内存使用**: ~200KB RAM
- **图像缓冲**: 67.68KB (3个缓冲区)

## 🤝 贡献指南

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 📝 开发日志

### v0.1 (当前版本)
- ✅ 基础框架搭建
- ✅ 多核通信机制
- ✅ 摄像头驱动
- ✅ 计算机视觉算法
- ✅ 运动控制系统
- 🔄 传感器融合优化
- 🔄 通信协议完善

## 📄 许可证

本项目采用 Boost Software License - Version 1.0 许可证。

## 👥 团队成员

- **项目负责人**: 吴宗桓
- **算法开发**: 廖宇辉
- **硬件设计**: Duffy
- **系统集成**: 芯动计划团队

## 📞 联系方式

- **项目地址**: https://git.tsinghua.edu.cn/wu-zh20/XinDong2025Code
- **技术支持**: 通过GitLab Issues提交问题
- **团队邮箱**: [联系邮箱]

---

**芯动计划2025** - 让智能车驰骋未来 🚗⚡

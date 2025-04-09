# Mini Probe

[English](./README.md)

![](./mini-probe.jpg)

Mini-Probe 是一个开源项目，旨在通过电子屏幕实时显示家中设备（如 NAS）的运行状态，使无显示器的设备也能直观呈现关键信息。

## 功能特性

- **实时监控**：展示设备的运行时间、内存使用、网络流量、磁盘使用等信息。
- **硬件兼容**：支持 Arduino UNO、NodeMCU 等开发板，以及 IIC 2004 LCD、IIC 1602 LCD 等液晶屏。
- **动态更新**：通过指令动态刷新屏幕内容，支持信息滚动显示，适应超出屏幕宽度的内容。

## 硬件要求

- **开发板**：Arduino UNO、NodeMCU 或其他兼容的微控制器。
- **液晶屏**：IIC 2004 LCD 或 IIC 1602 LCD。
- **连接线**：用于连接开发板和液晶屏的数据线。

## 软件要求

- **Arduino IDE**：用于编写和上传固件代码。
- **Docker**：被监控设备需支持 Docker，以便运行数据采集服务。

## 安装与配置

1. **硬件连接**：
   - 将液晶屏通过 I2C 接口连接到开发板。
   - 确保连接正确，SDA 对应 A4 引脚，SCL 对应 A5 引脚（以 Arduino UNO 为例）。

2. **固件上传**：
   - 在 Arduino IDE 中打开 `lcd-info-scroll.ino` 文件。
   - 选择对应的开发板型号和端口。
   - 点击“上传”将固件烧录到开发板。

3. **数据采集服务部署**：
   - 在被监控设备上，使用 Docker 部署数据采集服务，具体步骤参考项目中的 `docker-compose.yml` 文件。
   - 确保服务正常运行，并能与开发板通信。

## 使用说明

- 启动开发板后，液晶屏将自动显示设备的运行状态信息。
- 对于超出屏幕宽度的内容，系统会自动滚动显示。
- 如需修改显示内容或刷新频率，可在固件代码中进行调整。

## 参考资料

- 项目源码：[https://github.com/soulteary/mini-probe](https://github.com/soulteary/mini-probe)
- 相关教程：[把状态“亮”出来：几十块搞定家用 NAS 电子监控屏](https://soulteary.com/2025/04/09/light-up-the-status-a-few-costs-to-get-home-nas-electronic-monitoring-screen.html)

## 许可证

本项目基于 GPL-3.0 许可证开源，详细信息请参阅 LICENSE 文件。

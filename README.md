Bluetooth split keyboard

## 双轨迹球（charybdis36-studio）

主控为 nice!nano v2，右手连接电脑，左手通过 BLE 向右手转发按键和轨迹球事件。
两侧 PMW3610 接线相同：

| 信号 | Pro Micro 编号 | nRF52840 GPIO |
| --- | --- | --- |
| SCK | D0 | P0.08 |
| SDIO（MOSI/MISO 共线） | D2 | P0.17 |
| CS | D3 | P0.20 |
| MOTION | D1 | P0.06 |

矩阵使用 D10/D21/D20/D19 和 D4–D9，与上述引脚不重叠。
传感器定义集中在 `config/boards/shields/chary/chary_pointing.dtsi`，
两侧驱动开关放在 `config/chary.conf`。不要再添加覆盖这些引脚的
`config/chary_right.overlay`。

PMW3610 驱动放在 `config/modules/pmw3610`，兼容当前 Zephyr 3.5 和分体从机。
驱动只上报原始位移，左手用 `zmk,input-split` 发送到右手，右手的两个
listener 统一处理方向、速度和按键层：默认移动；层 1 滚动；移动时临时
开启层 2，停止 400 ms 后退出。移动缩放保留余数，避免小位移被整除丢弃。
上电初始化额外等待 1000 ms，让 nice!nano 的 VCC 稳定。

## 构建、刷入与验证

GitHub Actions 按 `build.yaml` 分别生成左右手 UF2。本地 west 工作区执行：

```sh
west update
west build -p -s zmk/app -d build/left -b nice_nano_v2 -S zmk-usb-logging -- -DSHIELD=chary_left -DZMK_CONFIG=/绝对路径/Charybdis46/config
west build -p -s zmk/app -d build/right -b nice_nano_v2 -S 'studio-rpc-usb-uart zmk-usb-logging' -- -DSHIELD=chary_right -DZMK_CONFIG=/绝对路径/Charybdis46/config -DCONFIG_ZMK_STUDIO=y
```

左右手必须刷入同一次构建的对应固件。先通过右手 USB 测试右球，再让左手
通过 BLE 连接右手测试左球，最后测试层 1 滚动和层 2 鼠标按键。

普通固件保留右手 Studio RPC。诊断固件通过 USB 日志串口记录矩阵扫描。
启动 PMW3610 后应出现 `PMW3610 initialized`。若出现 `Failed self-test`、
`Incorrect product id` 或 `PMW3610 initialization failed`，检查对应侧
VCC/GND、SPI 接线和焊接。若右球正常、左侧按键正常但左球无事件，先确认
两侧都已刷入新固件；仍失败时再检查分体 BLE 的输入服务发现和日志。


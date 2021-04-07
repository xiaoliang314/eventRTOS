# eventRTOS
* 抢占式内核CODE仅406B，RAM仅36B，无栈开销，在RAM < 512B的MCU依旧流畅运行。
* ARM Systick驱动，支持Clock级高精度定时器，CODE增加仅650B，RAM增加仅16B
* 每增加一个可抢占事件(任务)，RAM增加仅16B
* 支持0-255个抢占优先级
* 支持无栈协程

### 高精度软件定时器效果图：
<img src="定时器效果图.png">
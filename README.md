# eventRTOS
* 抢占式内核CODE大小406B，RAM开销36B，无栈开销。
* ARM Systick驱动，支持Clock级高精度定时器，CODE增加650B，RAM增加16B
* 每增加一个可抢占事件(任务)，RAM增加16B
* 支持0-255个抢占优先级
* 支持无栈协程

### 高精度软件定时器效果图：
<img src="定时器效果图.png">

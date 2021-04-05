/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __DRIVERS_TIMER_H__
#define __DRIVERS_TIMER_H__

#include <os/time.h>

/* 获取滴答时间 */
extern ktime_tick_t  drv_ktime_tick_get(void);

/*
 * tick单位转换，为了避免多余的乘除法运算，则由驱动实现us和ms对tick的转换
 */
extern ktime_ms_t    drv_ktime_tick_to_ms(ktime_tick_t tick);
extern ktime_us_t    drv_ktime_tick_to_us(ktime_tick_t tick);
extern ktime_tick_t  drv_ktime_us_to_tick(ktime_us_t us);
extern ktime_tick_t  drv_ktime_ms_to_tick(ktime_ms_t ms);

/* 定时器设置到期时间，expiry为0值表示永不到期
 * 当时间到达时调用sys_ktimer_timeout_check
 */
extern void drv_ktimer_set_expiry(ktime_tick_t expiry);


#endif /* __DRIVERS_TIMER_H__ */

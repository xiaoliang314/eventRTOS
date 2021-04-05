/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __OS_KTIMER_H__
#define __OS_KTIMER_H__

#include <os/time.h>
#include <os/kevent.h>
#include <drivers/timer_port.h>

typedef struct ktimer_event_s {
    kevent_t event;

    /* 定时器到期的时间 */
    ktime_tick_t expiry;
} ktimer_event_t;

#define KTIMER_EVENT_STATIC_INIT(ktimer, ktimer_cb, cb_data, priority)  \
{                                                                       \
    KEVENT_STATIC_INIT((ktimer).event, ktimer_cb, cb_data, priority), 0 \
}

/* 定时器的节点 */
#define KTIMER_NODE(timer)   (KEVENT_NODE(&(timer)->event))

/* 节点上的定时器 */
#define KTIMER_OF_NODE(node) ((ktimer_event_t *)KEVENT_OF_NODE(node))

/* 定时器超时检查，可能触发定时器完成事件 */
void sys_ktimer_timeout_check(ktime_tick_t now);

/* 获取定时器中最早的到期时间，返回0则表示无定时器 */
ktime_tick_t sys_ktimer_earliest_expiry(void);

/***********************************
 * 定时器API
 ***********************************/

#define ktime_tick_get      drv_ktime_tick_get
#define ktime_tick_to_ms    drv_ktime_tick_to_ms
#define ktime_tick_to_us    drv_ktime_tick_to_us
#define ktime_us_to_tick    drv_ktime_us_to_tick
#define ktime_ms_to_tick    drv_ktime_ms_to_tick

/* 初始化定时器 */
static force_inline void ktimer_init(ktimer_event_t *timer,
        kevent_cb ecb,
        void *ctx,
        uint8_t priority)
{
    kevent_init(&timer->event, ecb, ctx, priority);
}

static force_inline void ktimer_init_inhert(ktimer_event_t *timer, kevent_t *parent)
{
    kevent_init_inherit(&timer->event, parent);
}

/* 启动定时器，在expiry时将会触发 */
void ktimer_start_expiry(ktimer_event_t *timer, ktime_tick_t expiry);

/* 取消定时器 */
void ktimer_stop(ktimer_event_t *timer);

/* 启动定时器，在指定时间后触发定时器 */
static force_inline void ktimer_start_ms(ktimer_event_t *timer, ktime_us_t timeout_ms)
{
    ktimer_start_expiry(timer, ktime_tick_get() + ktime_ms_to_tick(timeout_ms));
}

static force_inline void ktimer_start_us(ktimer_event_t *timer, ktime_us_t timeout_us)
{
    ktimer_start_expiry(timer, ktime_tick_get() + ktime_us_to_tick(timeout_us));
}

/* 获取定时器到期时刻 */
static force_inline ktime_tick_t ktimer_expiry_get(ktimer_event_t *timer)
{
    return timer->expiry;
}

#endif /* __OS_TIMER_H__ */

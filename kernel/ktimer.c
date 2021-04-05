/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#include <os/ktimer.h>
#include <arch/irq.h>

/* 定时器队列 */
static fifo_t timers = FIFO_STATIC_INIT(timers);

/* 启动定时器，在expiry时将会触发 */
void ktimer_start_expiry(ktimer_event_t *timer, ktime_tick_t expiry)
{
    int key;
    ktimer_event_t *find;
    slist_node_t *prev_node;
    slist_node_t *cur_node;
    
    expiry = expiry == 0 ? 1 : expiry;

    key = irq_lock();

    /* ktimer处于队列中，无法被重复插入 */
    if (!slist_node_is_del(KTIMER_NODE(timer))) {
        irq_unlock(key);
        return;
    }

    timer->expiry = expiry;

    /* 先查看是否可以插入到队列尾部 */
    find = KTIMER_OF_NODE(FIFO_TAIL(&timers));
    if (fifo_is_empty(&timers) || find->expiry <= expiry) {
        fifo_push(&timers, KTIMER_NODE(timer));
    } else {
        /* 遍历定时器列表，将定时器插入到合适的位置 */
        slist_foreach_record_prev(FIFO_LIST(&timers), cur_node, prev_node) {
            find = KTIMER_OF_NODE(cur_node);
            if (find->expiry > expiry) {
                fifo_node_insert_next(&timers, prev_node, KTIMER_NODE(timer));
                break;
            }
        }
    }

    /* 若timer被插到头部，则更新到期时间 */
    if (KTIMER_NODE(timer) == FIFO_TOP(&timers)) {
        drv_ktimer_set_expiry(expiry);
    }

    irq_unlock(key);
}

void sys_ktimer_timeout_check(ktime_tick_t now)
{
    ktime_tick_t expiry;
    ktimer_event_t *timer;
    int key;
    bool update = false;

    key = irq_lock();

    while (!fifo_is_empty(&timers)) {
        timer = KTIMER_OF_NODE(FIFO_TOP(&timers));
        /* 未超时 */
        if (now < timer->expiry) {
            break;
        }
        update = true;

        fifo_pop(&timers);
        irq_unlock(key);

        /* 提交这个定时器事件 */
        kevent_post(&timer->event);

        irq_lock();
    }

    if (update) {
        expiry = 0;
        if (!fifo_is_empty(&timers)) {
            expiry = KTIMER_OF_NODE(FIFO_TOP(&timers))->expiry;
        }

        drv_ktimer_set_expiry(expiry);
    }

    irq_unlock(key);
}

/* 获取定时器中最早的到期时间 */
ktime_tick_t sys_ktimer_earliest_expiry(void)
{
    ktime_tick_t expiry = 0;
    int key = irq_lock();

    if (!fifo_is_empty(&timers)) {
        expiry = KTIMER_OF_NODE(FIFO_TOP(&timers))->expiry;
    }

    irq_unlock(key);
    return expiry;
}

/* 取消定时器 */
void ktimer_stop(ktimer_event_t *timer)
{
    ktime_tick_t new_expiry;
    slist_node_t *top;
    int key = irq_lock();

    if (slist_node_is_del(KTIMER_NODE(timer))) {
        goto exit;
    }

    if (kevent_is_ready(&timer->event)) {
        kevent_cancel(&timer->event);
        goto exit;
    }

    top = FIFO_TOP(&timers);
    fifo_del_node(&timers, KTIMER_NODE(timer));

    if (top == KTIMER_NODE(timer)) {
        new_expiry = 0;
        if (!fifo_is_empty(&timers)) {
            new_expiry = KTIMER_OF_NODE(FIFO_TOP(&timers))->expiry;
        }

        drv_ktimer_set_expiry(new_expiry);
    }

exit:
    irq_unlock(key);
}

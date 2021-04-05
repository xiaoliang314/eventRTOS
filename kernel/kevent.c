/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#include <os/kevent.h>
#include <arch/irq.h>

typedef struct kevent_scheduler_s {
    fifo_t ready_groups[KEVENT_PRIORITY_GROUP_COUNT];

    uint8_t ready_map;

    uint8_t schedule_pending;

    int16_t scheduling_priority;
} kevent_scheduler_t;


static kevent_scheduler_t scheduler = {
    {
        FIFO_STATIC_INIT(scheduler.ready_groups[0]),
        FIFO_STATIC_INIT(scheduler.ready_groups[1]),
        FIFO_STATIC_INIT(scheduler.ready_groups[2]),
        FIFO_STATIC_INIT(scheduler.ready_groups[3])
    },
    0,
    0,
    -1
};


void kevent_fifo_priority_push(fifo_t *epfifo, kevent_t *event)
{
    kevent_t *insert_pos = KEVENT_OF_NODE(FIFO_TAIL(epfifo));
    slist_node_t *prev_node, *node;

    /* 如果队列空或者当前事件优先级低于队尾优先级，则将事件插入到尾部 */
    if (fifo_is_empty(epfifo) || 
        KEVENT_PRIORITY(event) <= KEVENT_PRIORITY(insert_pos)) {
        fifo_push(epfifo, KEVENT_NODE(event));
    } else {
        /* 从头按优先级查找插入的位置 */
        slist_foreach_record_prev(FIFO_LIST(epfifo), node, prev_node) {
            insert_pos = container_of(node, kevent_t, node);

            if (KEVENT_PRIORITY(event) > KEVENT_PRIORITY(insert_pos)) {
                fifo_node_insert_next(epfifo, prev_node, KEVENT_NODE(event));
                break;
            }
        }
    }
}


void kevent_post(kevent_t *e)
{
    uint8_t ready_group;
    uint8_t priority;
    int key;

    if (e->priority == KEVENT_PRIORITY_IMMED) {
        e->callback(e->cb_data, e);
        return;
    }

    key = irq_lock();

    /* 事件节点必须处于空闲状态 */
    if (slist_node_is_del(KEVENT_NODE(e))) {
        priority = e->priority;
        ready_group = priority >> KEVENT_READY_GROUP_PRIORITY_SHIFT;

        /* 添加事件到相应的事件组 */
        kevent_fifo_priority_push(&scheduler.ready_groups[ready_group], e);

        e->is_ready = 1;
        scheduler.ready_map |= (1 << ready_group);

        /* 若提交的事件大于正在调度事件的优先级，则挂起抢占 */
        if (priority > scheduler.scheduling_priority &&
            !scheduler.schedule_pending) {
            scheduler.schedule_pending = 1;
            arch_irq_schedule_pending();
        }
    }

    irq_unlock(key);
}


void kevent_cancel(kevent_t *e)
{
    uint8_t ready_group;
    fifo_t *ready_q;
    int key;

    key = irq_lock();

    ready_group = e->priority >> KEVENT_READY_GROUP_PRIORITY_SHIFT;
    ready_q = &scheduler.ready_groups[ready_group];

    /* 节点非空闲状态则从事件组中删除 */
    if (!slist_node_is_del(KEVENT_NODE(e))
        && fifo_del_node(ready_q, KEVENT_NODE(e))) {
        /* 若事件组为空，则更新就绪图 */
        if (fifo_is_empty(ready_q)) {
            scheduler.ready_map &= ~(1 << ready_group);
        }

        e->is_ready = 0;
    }

    irq_unlock(key);
}

static force_inline uint8_t scheduler_highest_ready_group_get(uint8_t ready_map)
{
    static const uint8_t priority_ready_bitmap[] = {
    /*     000, 001, 010, 011, 100, 101, 110, 111 */
    /* 0 */0xFF, 0,   1,   1,   2,   2,   2,   2,
    /* 1 */3,    3,   3,   3,   3,   3,   3,   3
    };

    return priority_ready_bitmap[ready_map];
}

void kevent_schedule(void)
{
    kevent_t *e;
    fifo_t *ready_q;
    uint8_t ready_group;
    int32_t old_scheduling_priority;
    uint8_t priority;
    int key;

    key = irq_lock();
    /* 保存前一次正在调度的优先级，以便后续恢复 */
    old_scheduling_priority = scheduler.scheduling_priority;

    /* 清除调度挂起标识 */
    scheduler.schedule_pending = 0;

    while (1) {
        /* 获取当前最高优先级的事件组 */
        ready_group = scheduler_highest_ready_group_get(scheduler.ready_map);
        if (ready_group >= KEVENT_PRIORITY_GROUP_COUNT) {
            break;
        }

        /* 获取最高优先级的事件 */
        ready_q = &scheduler.ready_groups[ready_group];
        e = KEVENT_OF_NODE(FIFO_TOP(ready_q));
        priority = e->priority;

        /* 只调度比当前优先级更高的事件 */
        if (priority <= old_scheduling_priority) {
            break;
        }

        /* 取出这个事件以执行调度 */
        fifo_pop(ready_q);
        /* 若事件为空，则清除该组就绪map */
        if (fifo_is_empty(ready_q)) {
            scheduler.ready_map &= ~(1UL << ready_group);
        }

        e->is_ready = 0;
        /* 设置正在调度的优先级 */
        scheduler.scheduling_priority = priority;

        /* 打开中断并调度这个事件 */
        irq_unlock(key);
        e->callback(e->cb_data, e);
        key = irq_lock();
    }

    /* 恢复前一次调度优先级 */
    scheduler.scheduling_priority = old_scheduling_priority;

    irq_unlock(key);
}

bool kevent_scheduler_busy(void)
{
    return scheduler.ready_map;
}

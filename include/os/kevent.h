/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __OS_KEVENT_H__
#define __OS_KEVENT_H__

#include <bases.h>
#include <fifo.h>
#include <lifo.h>

typedef struct kevent_s {
    slist_node_t node;

    void (*callback)(void *cb_data, struct kevent_s *e);

    void *cb_data;

    uint8_t priority;

    uint8_t is_ready;

    uint8_t bp;

    uint8_t unused;
} kevent_t;

/************************************************************
 *@简介：
 ***事件结构体静态初始化
 *
 *@用法：
 ***kevent_t event = KEVENT_STATIC_INIT(event);
 *
 *@参数：
 *[event]：事件变量名，非地址
 *[callback]：事件回调函数
 *[ctx]：事件的回调上下文
 *[priority]：事件的优先级
 *************************************************************/
#define KEVENT_STATIC_INIT(event, callback, cb_data, priority) \
{                                               \
    SLIST_NODE_STATIC_INIT((event).node),       \
    (callback),                                 \
    (cb_data),                                  \
    (priority), 0, 0                            \
}

/* 事件回调类型定义 */
typedef void (*kevent_cb)(void *, kevent_t *);

/*********************************************************
 *@说明：
 ***事件相关成员变量引用
 *********************************************************/
/* 事件的节点 */
#define KEVENT_NODE(event)   ((slist_node_t *)(event))

/* 节点上的事件 */
#define KEVENT_OF_NODE(_node) ((kevent_t *)(_node))

/* 事件的上下文 */
#define KEVENT_CONTEXT(event) ((event)->context)

/* 事件的回调 */
#define KEVENT_CALLBACK(event) ((event)->callback)

/* 事件的优先级 */
#define KEVENT_PRIORITY(event) ((event)->priority)

/* 事件已被引用或者处于队列之中 */
#define kevent_is_ref(event)    slist_node_is_ref(&(event)->node)

/************************************************************
 *@简介：
 ***事件初始化
 *
 *@参数：
 *[event]：被初始化的事件指针
 *[callback]：事件回调函数
 *[ctx]：事件的回调上下文
 *[priority]：事件的优先级
 *************************************************************/
static force_inline void kevent_init(kevent_t *event,
        kevent_cb ecb,
        void *ctx,
        uint8_t priority)
{
    event->priority = priority;
    event->is_ready = 0;
    event->bp = 0;
    event->cb_data = ctx;
    event->callback = ecb;
    slist_node_init(&event->node);
}


/*****************************************
 *@简要：
 ***事件继承初始化，将继承父事件的回调函数，上下文，优先级
 *
 *@参数
 *[event]：被初始化的事件指针
 *[parent]：父事件
 *****************************************/
static inline void kevent_init_inherit(kevent_t *event, const kevent_t *parent)
{
    event->priority = parent->priority;
    event->is_ready = 0;
    event->cb_data = parent->cb_data;
    event->callback = parent->callback;
    slist_node_init(&event->node);
}

enum
{
    /* 立即事件：被提交便立即执行，优先级等于调用者优先级，可以在中断上下文中执行 */
    KEVENT_PRIORITY_IMMED = 0xff,

    /* 最高优先级组 */
    KEVENT_PRIORITY_HIGHEST_GROUP = 0xc0,

    /* 高优先级组 */
    KEVENT_PRIORITY_HIGH_GROUP = 0x80,

    /* 中优先级 */
    KEVENT_PRIORITY_MIDDLE_GROUP = 0x40,

    /* 低优先级组 */
    KEVENT_PRIORITY_LOWER_GROUP = 0x00,

    /* 优先级组个数 */
    KEVENT_PRIORITY_GROUP_COUNT = 4
};

/* 优先级位字段定义 */
#define KEVENT_READY_SUB_PRIORITY_MASK      0x3F
#define KEVENT_READY_GROUP_PRIORITY_MASK    0xC0
#define KEVENT_READY_GROUP_PRIORITY_SHIFT   6

/*********************************************************
*@简要：
***检查事件是否已经就绪(将被调度)
*
*@约定：
***不能使用空指针
*
*@参数：
*[e]：事件
*
*@返回值：
*[true]：已就绪
*[false]：未就绪
**********************************************************/
static force_inline bool kevent_is_ready(kevent_t *e)
{
    return e->is_ready != 0;
}

/*
 * epfifo为事件队列，并按事件优先级插入事件
 */
void kevent_fifo_priority_push(fifo_t *epfifo, kevent_t *event);

/*********************************************************
*@简要：
***向调度器提交一个事件
*
*@约定：
***不能使用空指针
*
*@参数：
*[e]：被提交的事件
*
**********************************************************/
void kevent_post(kevent_t *e);

/*********************************************************
*@简要：
***取消调度器中的一个事件
*
*@约定：
***不能使用空指针
*
*@参数：
*[e]：被提交的事件
*
**********************************************************/
void kevent_cancel(kevent_t *e);

/*********************************************************
*@简要：
***调度事件队列中比当前更高优先级的事件
**********************************************************/
void kevent_schedule(void);

/*********************************************************
*@简要：
***判断事件调度器是否处于busy状态
**********************************************************/
bool kevent_scheduler_busy(void);

#endif /* __OS_KEVENT_H__ */

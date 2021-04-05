/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __OS_MSG_QUEUE_H__
#define __OS_MSG_QUEUE_H__

#include <os/kevent.h>

typedef struct kmsg_queue_s {
    /* 消息队列，按先进先出方式进队 */
    fifo_t msg_q;

    /* 通常只有一个处理者，因此不需要使用FIFO */
    lifo_t wait_q;
} kmsg_queue_t;

#define KMSG_QUEUE_STATIC_INIT(kmsg_q)  \
{ FIFO_STATIC_INIT((kmsg_q).msg_q), LIFO_STATIC_INIT((kmsg_q).wait_q) }

static inline void kmsg_queue_init(kmsg_queue_t *kmsg_q)
{
    fifo_init(&kmsg_q->msg_q);
    lifo_init(&kmsg_q->wait_q);
}


/* 添加消息至消息队列
 * 若消息队列存在监听事件，该操作结束后将触发监听事件
 */
void kmsg_queue_push(kmsg_queue_t *kmsg_q, slist_node_t *msg);

/* 
 * 从消息队列中取出消息
 * 传入一个监听事件，若队列非空返回队列头部的消息。若队列为空返回NULL，并设置监听事件
 */
slist_node_t *kmsg_queue_pop(kmsg_queue_t *kmsg_q, kevent_t *listen_ev);

#endif /* __OS_MSG_QUEUE_H__ */

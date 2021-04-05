/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __OS_SLAB_MEM_H__
#define __OS_SLAB_MEM_H__

#include <os/kevent.h>
#include <arch/irq.h>

typedef struct kslab_mem_s {
    /* 空闲内存链表 */
    lifo_t free_list;

    /* slab唤醒队列 */
    fifo_t wait_q;
} kslab_mem_t;

/* slab事件 */
typedef struct kslab_event_s {
    kevent_t event;
    void *mem_blk;
} kslab_event_t;

/* slab事件初始化 */
#define KSLAB_EVENT_STATIC_INIT(kslab_event, ecb, cb_data, priority)    \
{                                                                       \
    KEVENT_STATIC_INIT((kslab_event).event, ecb, cb_data, priority),    \
    0                                                                   \
}

#define kslab_event_init(slab_event, ecb, ctx, priority)            \
    do                                                              \
    {                                                               \
        kevent_init(&(slab_event)->event, (ecb), (ctx), (priority));\
        (slab_event)->mem_blk = 0;                                  \
    } while (0)

#define kslab_event_init_inherit(slab_event, parent)                \
    do                                                              \
    {                                                               \
        kevent_init_inherit(&(slab_event)->event, (parent));        \
        (slab_event)->mem_blk = 0;                                  \
    } while (0)

/* kslab事件到节点的转换 */
#define KSLAB_EVENT_EVENT(slab_event)   (&(slab_event)->event)
#define KSLAB_EVENT_NODE(slab_event)    KEVENT_NODE(&(slab_event)->event)
#define KSLAB_EVENT_OF_EVENT(event)    ((kslab_event_t*)(event))
#define KSLAB_EVENT_OF_NODE(node)      KSLAB_EVENT_OF_EVENT(KEVENT_OF_NODE(node))

/*********************************************
 *@简要：使用buffer初始化一个slab分配器
 *
 *@参数：
 *[slab]	 slab
 *[buff]     buffer
 *[blk_nums] buffer的块数
 *[blk_size] 每块的大小
 *********************************************
 */
void kslab_mem_init(kslab_mem_t *slab, void *buff, uint32_t blk_nums, uint32_t blk_size);

/*********************************************
 *@简要：使用数组初始化一个kslab分配器
 *
 *@参数：
 *[slab]	slab
 *[arr]     数组
 *********************************************
 */
#define kslab_mem_init_by_arr(slab, arr)    kslab_mem_init((slab), (arr), ARRAY_SIZE(arr), sizeof(*(arr)))

/*********************************************
 *@简要：slab分配器分配内存块
 *
 *@参数：
 *[slab] slab分配器
 *
 *@返回： 内存块
 *********************************************
 */
static inline void *kslab_mem_alloc(kslab_mem_t *slab)
{
    int key = irq_lock();
    void *mem = 0;

    if (!lifo_is_empty(&slab->free_list)) {
        mem = lifo_pop(&slab->free_list);
    }

    irq_unlock(key);
    return mem;
}


/*********************************************
 *@简要：slab分配器释放内存块
 *
 *@参数：
 *[slab] slab分配器
 *[mem] 内存块
 *
 *@返回：无
 *********************************************
 */
void kslab_mem_free(kslab_mem_t *slab, void *mem);

/*********************************************
 *@简要：slab分配器静默释放内存块
 *
 *@参数：
 *[slab] slab分配器
 *[mem] 内存块
 *
 *@返回：无
 *********************************************
 */
static force_inline void kslab_mem_free_quiet(kslab_mem_t *slab, void *mem)
{
    int key = irq_lock();

    /* 将节点插入队列 */
    slist_node_insert_next(SLIST_HEAD(&slab->free_list), (slist_node_t *)mem);

    irq_unlock(key);
}

/*********************************************
 *@简要：等待slab分配器内存块可用
 *
 *@参数：
 *[slab] slab分配器
 *
 *********************************************
 */
void kslab_mem_wait(kslab_mem_t *slab, kslab_event_t *slab_event);

#endif /* __OS_SLAB_MEM_H__ */

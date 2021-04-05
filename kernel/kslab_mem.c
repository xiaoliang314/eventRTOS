/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#include <os/slab_mem.h>

void kslab_mem_init(kslab_mem_t *slab, void *buff, uint32_t blk_nums, uint32_t blk_size)
{
    uint8_t *free_node;
    uint32_t i;

	fifo_init(&slab->wait_q);

	/* 初始化空闲块链表 */
	lifo_init(&slab->free_list);

	/* generates a free block list */
	/* 生成空闲块链表 */
	free_node = buff;
	for (i = 0; i < blk_nums; i++) {
		lifo_push(&slab->free_list, (slist_node_t *)free_node);
		free_node += blk_size;
	}
}

void kslab_mem_wait(kslab_mem_t *slab, kslab_event_t *slab_event)
{
    int key = irq_lock();

    /* 将事件添加到等待列表 */
    if (!kevent_is_ref(&slab_event->event)) {
        kevent_fifo_priority_push(&slab->wait_q, KSLAB_EVENT_EVENT(slab_event));

        /* 若有内存可用，则唤醒等待队列中的一个事件 */
        if (!lifo_is_empty(&slab->free_list)) {
            slab_event = KSLAB_EVENT_OF_NODE(fifo_pop(&slab->wait_q));
            slab_event->mem_blk = slist_node_del_next(SLIST_HEAD(&slab->free_list));
            irq_unlock(key);

            kevent_post(&slab_event->event);
        } else {
            irq_unlock(key);
        }
    }

    irq_unlock(key);
}

void kslab_mem_free(kslab_mem_t *slab, void *mem)
{
    kslab_event_t *slab_event;
    int key = irq_lock();

    /* 通知等待者slab已可用 */
    if (!fifo_is_empty(&slab->wait_q)) {
        slab_event = KSLAB_EVENT_OF_EVENT(fifo_pop(&slab->wait_q));
        slab_event->mem_blk = mem;
        irq_unlock(key);

        kevent_post(&slab_event->event);
    } else {
        /* 将节点插入队列 */
        slist_node_insert_next(SLIST_HEAD(&slab->free_list), (slist_node_t *)mem);
        irq_unlock(key);
    }
}


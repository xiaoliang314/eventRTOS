/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#include <os/kernel.h>

void kmsg_queue_push(kmsg_queue_t *kmsg_q, slist_node_t *msg)
{
    int key;
    kevent_t *listen_ev;

    key = irq_lock();

    if (slist_node_is_del(msg)) {
        fifo_push(&kmsg_q->msg_q, msg);

        if (!lifo_is_empty(&kmsg_q->wait_q)) {
            listen_ev = KEVENT_OF_NODE(lifo_pop(&kmsg_q->wait_q));
            kevent_post(listen_ev);
        }
    }

    irq_unlock(key);
}

slist_node_t *kmsg_queue_pop(kmsg_queue_t *kmsg_q, kevent_t *listen_ev)
{
    int key;
    slist_node_t *node;

    key = irq_lock();

    if (fifo_is_empty(&kmsg_q->msg_q)) {
        node = NULL;
        if (listen_ev && !kevent_is_ref(listen_ev)) {
            lifo_push(&kmsg_q->wait_q, KEVENT_NODE(listen_ev));
        }
    } else {
        node = fifo_pop(&kmsg_q->msg_q);
    }

    irq_unlock(key);
    return node;
}

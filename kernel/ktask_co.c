/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#include <os/ktask_co.h>

void ktask_co_init(ktask_co_t *task, void *stack, size_t stack_size, uint8_t priority)
{
    kevent_init(&task->event, (kevent_cb)0, task, priority);

    task->stack.start = (uint8_t *)ALIGN_CPU_UP((size_t)stack);
    task->stack.end = task->stack.start + _SUB_BEZ(stack_size, (size_t)(task->stack.start - (uint8_t *)stack));
    task->stack.cur = task->stack.start;

    task->ret_val.ptr = NULL;

    task->cur_ctx.stack_used = 0;
    task->cur_ctx.bp = 0;
    task->cur_ctx.yield_state = 0;
    lifo_init(&task->task_end_notify_q);
}

void ktask_co_asyn_return(ktask_co_t *task)
{
    lifo_t end_notify_q;
    int key;

    if (task->stack.cur >= task->stack.start + KTASK_CO_STACK_CTX_SIZE) {
        /* 恢复调用者上下文信息及事件回调 */
        task->stack.cur -= KTASK_CO_STACK_CTX_SIZE;
        task->cur_ctx = *(struct ktask_co_cur_ctx_s *)task->stack.cur;
        KEVENT_CALLBACK(&task->event) = *(kevent_cb *)(task->stack.cur + sizeof(struct ktask_co_cur_ctx_s));
        task->stack.cur -= task->cur_ctx.stack_used;

        KTASK_CO_ASSERT(task->stack.cur >= task->stack.start && task->stack.cur <= task->stack.end);

        if (task->cur_ctx.yield_state) {
            /* 立即返回调用者 */
            task->event.callback(task->event.cb_data, &task->event);
        }
    }
    /* 任务结束 */
    else {
        task->stack.cur = task->stack.start;
        task->ret_val.ptr = NULL;
        task->cur_ctx.stack_used = 0;
        task->cur_ctx.bp = 0;
        task->cur_ctx.yield_state = 0;
        KEVENT_CALLBACK(&(task)->event) = (kevent_cb)0;

        key = irq_lock();
        lifo_nodes_transfer_to(&task->task_end_notify_q, &end_notify_q);
        irq_unlock(key);

        while (!lifo_is_empty(&end_notify_q)) {
            /* 异步提交事件可避免在回调中释放task而引起错误 */
            kevent_post(KEVENT_OF_NODE(lifo_pop(&end_notify_q)));
        }
    }
}

bool ktask_end_wait_cancel(ktask_co_t *task, kevent_t *ev)
{
    bool res;
    int key = irq_lock();

    if (slist_node_is_del(KEVENT_NODE(ev)) ||
        kevent_is_ready(ev)) {
        irq_unlock(key);
        return false;
    }

    res = lifo_del_node(&task->task_end_notify_q, KEVENT_NODE(ev));
    irq_unlock(key);
    return res;
}

void ktask_co_asyn_call_prepare(ktask_co_t *task, ktask_co_asyn_routine_t afunc, void **pbpd)
{
    KTASK_CO_ASSERT(task->stack.cur + task->cur_ctx.stack_used + KTASK_CO_STACK_CTX_SIZE <= task->stack.end);

    /* Save current context information and event callback */
    /* 保存当前上下文信息以及事件回调 */
    task->stack.cur += task->cur_ctx.stack_used;
    *pbpd = task->stack.cur;
    *(struct ktask_co_cur_ctx_s *)task->stack.cur = task->cur_ctx;
    *(kevent_cb *)(task->stack.cur + sizeof(struct ktask_co_cur_ctx_s)) = KEVENT_CALLBACK(&task->event);
    task->stack.cur += KTASK_CO_STACK_CTX_SIZE;

    /* Initialize new context information and event callback */
    /* 初始化新的上下文信息和事件回调 */
    task->cur_ctx.bp = 0;
    task->cur_ctx.stack_used = 0;
    task->cur_ctx.yield_state = 0;
    KEVENT_CALLBACK(&task->event) = (kevent_cb)afunc;
}

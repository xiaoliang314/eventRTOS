/*
 * Copyright (C) 2018-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __OS_KTASK_CO_H__
#define __OS_KTASK_CO_H__

#include <os/kevent.h>
#include <arch/irq.h>
#include <bp.h>

/*********************************************************
 *@类型说明：
 *
 *[struct task_cur_ctx_s]：task当前的上下文信息
 *[struct task_stack_s]：task当前栈指针信息
 *[task_t]：task数据结构
 *********************************************************/
struct ktask_co_cur_ctx_s
{
    uint8_t yield_state;
    uint8_t bp;
    uint16_t stack_used;
};

struct ktask_co_stack_s
{
    uint8_t *start;
    uint8_t *end;
    uint8_t *cur;
};

/* ktask_co协程任务 */
typedef struct ktask_co_s
{
    kevent_t event;
    struct ktask_co_stack_s stack;
    struct ktask_co_cur_ctx_s cur_ctx;
    union {
        void *ptr;
        uint32_t u32;
        int32_t  s32;
    } ret_val;

    lifo_t task_end_notify_q;
} ktask_co_t;

/* KTASK_CO错误断言 */
#ifndef KTASK_CO_ASSERT
#define KTASK_CO_ASSERT(expr)
#endif /* TASK_ASSERT */

/* KTASK_CO信息 */
#ifndef KTASK_CO_INFO
#define KTASK_CO_INFO(task, stack_size, stack_used)
#endif /* TASK_INFO */

/*********************************************************
 *@类型说明：
 *
 *[task_asyn_routine_t]：任务函数与异步函数类型
 *********************************************************/
typedef void (*ktask_co_asyn_routine_t)(struct ktask_co_s *, kevent_t *);

/* KTASK_CO的bpd指针，由bpd协程使用 */
#define KTASK_CO_BPD(task)  (&((ktask_co_t *)(task))->cur_ctx.bp)

/************************************************************
 *@简介：
 ***任务数据结构静态初始化
 *
 *@参数：
 *[task]：任务的变量名，非指针
 *[task_func]：任务函数
 *[stack]：任务的异步栈
 *[stack_size]：栈的大小
 *[priority]：任务事件的优先级
 *************************************************************/
#define KTASK_CO_STATIC_INIT(task, stack, stack_size, priority)                 \
{                                                                               \
    KEVENT_STATIC_INIT((task).event, (event_cb)NULL_CB, &task, (priority)),     \
    {                                                                           \
        (stack),                                                                \
        (uint8_t *)(stack) + (stack_size),                                      \
        (stack),                                                                \
    },                                                                          \
    {0, 0, BP_INIT_VAL},                                                        \
    {0},                                                                        \
    LIFO_STATIC_INIT((task).task_end_notify_q)                                  \
}


/************************************************************
 *@简介：
 ***定义一个任务对象，并初始化
 *
 *@参数：
 *[_name]：任务对象的名字
 *[task_func]：任务函数
 *[stack_size]：任务异步栈的大小
 *[priority]：任务事件的优先级
 *************************************************************/
#define KTASK_CO_DEFINE(_name, stack_size, priority)                \
    uint32_t _name##_stack_buf[(stack_size + 3) / 4];               \
    ktask_co_t  _name = TASK_STATIC_INIT(_name,                     \
                                     _name##_stack_buf,             \
                                     (stack_size),                  \
                                     (priority))


/************************************************************
 *@简介：
 ***使用static修饰定义一个任务对象，并初始化
 *
 *@参数：
 *[_name]：任务对象的名字
 *[task_func]：任务函数
 *[stack_size]：任务异步栈的大小
 *[priority]：任务事件的优先级
 *************************************************************/
#define KTASK_CO_DEFINE_STATIC(_name, stack_size, priority)         \
    static uint32_t _name##_stack_buf[(stack_size + 3) / 4];        \
    static ktask_co_t  _name = TASK_STATIC_INIT(_name,              \
                                            _name##_stack_buf,      \
                                            (stack_size),           \
                                            (priority))

/* 进行异步调用时需要保存的任务上下文信息大小 */
#define KTASK_CO_STACK_CTX_SIZE    (sizeof(struct ktask_co_cur_ctx_s) + sizeof(kevent_cb))

/* 简单的宏，返回a - b保证大于等于0 */
#define _SUB_BEZ(a, b)  ((a) > (b) ? ((a) - (b)) : 0)

/************************************************************
 *@简介：
 ***任务数据结构动态初始化，若task已静态初始化，则不必使用该函数
 *
 *@参数：
 *[task]：任务的变量名，非指针
 *[task_func]：任务函数
 *[stack]：任务的异步栈
 *[stack_size]：栈的大小
 *[priority]：任务事件的优先级
 *************************************************************/
void ktask_co_init(ktask_co_t *task, void *stack, size_t stack_size, uint8_t priority);


/************************************************************
 *@简要：
 ***将一个事件初始化为协程的异步事件
 *
 *@参数
 *[event]：被初始化的事件指针
 *[task]：协程
 *************************************************************/
#define ktask_co_event_inherit(event, task)          kevent_init_inherit(event, &(task)->event)


/************************************************************
 *@简介：
 ***启动任务
 *
 *@参数：
 *[task]：任务对象，不能为空
 *[task_func]: 任务函数，任务函数应该被声明为 void task_func(task_t *task, event_t *ev, 参数1, 参数2..)格式
 *[...]: 任务函数的第3个及之后的参数
 *************************************************************/
#define task_start(task, task_func, ...)                        \
    do {                                                        \
        KEVENT_CALLBACK(&(task)->event) = (event_cb)(task_func);\
        (task_func)((task), NULL, ##__VA_ARGS__);               \
    } while (0)


/*********************************************************
 *@简要：
 ***获取当前task堆栈中的异步变量
 ***
 ***task在首次调用时，将产生分配内存的操作，
 ***bpd_yield导致bp不为0后，再次获取，则返回以前的内存
 *
 *@约定：
 ***不能使用空指针
 *
 *@参数：
 *[task]：任务对象
 *[vars_size]：异步变量的大小
 *
 *@返回：异步变量的内存地址
 **********************************************************/
static inline void *ktask_co_asyn_vars_get(ktask_co_t *task, size_t vars_size)
{
    size_t alloc_size;

    if (task->cur_ctx.bp == 0) {
        alloc_size = ALIGN_CPU_UP(vars_size);

        KTASK_CO_INFO((task), (task->stack.end - task->stack.start), (task->stack.cur + alloc_size - task->stack.start));
        KTASK_CO_ASSERT(task->stack.cur + alloc_size <= task->stack.end);

        task->cur_ctx.stack_used = alloc_size;
    }

    return task->stack.cur;
}

/*********************************************************
 *@简要：
 ***被异步调用的函数返回到调用者
 ***
 ***任何由task_bpd_asyn_call调用的函数，应当使用此方法返回到调用者
 *
 *@约定：
 ***不能使用空指针
 *
 *@参数：
 *[task]：任务对象
 **********************************************************/
void ktask_co_asyn_return(ktask_co_t *task);


/*********************************************************
 *@简要：
 ***判断协程任务是否结束
 *
 *@返回：
 *[true]：任务已结束
 *[false]: 任务未结束
 **********************************************************/
static force_inline bool ktask_co_is_end(ktask_co_t *task)
{
    return KEVENT_CALLBACK(&(task)->event) == 0;
}

/*********************************************************
 *@简要：
 ***为协程任务的结束动作添加一个监听事件，协程任务结束时将触发
 ***监听的事件
 *
 *@约定：
 ***1、最上层的协程(由task_start启动)结束时必须调用task_asyn_return
 *
 *@参数：
 *[task]：任务对象
 *[task_end_notify_ev]：用于监听协程退出的事件
 *
 *@返回值：
 *[true]：添加监听事件成功
 *[false]：监听事件处于引用状态或者队列中
 **********************************************************/
static inline void ktask_co_end_wait(ktask_co_t *task, kevent_t *task_end_notify_ev)
{
    int key = irq_lock();

    if (!slist_node_is_del(KEVENT_NODE(task_end_notify_ev))) {
        irq_unlock(key);
    }

    lifo_push(&task->task_end_notify_q, KEVENT_NODE(task_end_notify_ev));
    irq_unlock(key);
}


/*********************************************************
 *@简要：
 ***取消task_end_wait的事件监听
 *
 *@参数：
 *[task]：任务对象
 *[task_end_notify_ev]：传入task_end_wait的事件
 *
 *@返回值：
 *[true]：取消监听事件成功
 *[false]：监听的事件已经不在监听队列之中
 **********************************************************/
bool ktask_end_wait_cancel(ktask_co_t *task, kevent_t *task_end_notify_ev);


/************************************************************
 *@简介：
 ***将当前上下文保存到栈中，并初始化新的上下文信息
 ***
 ***此函数由task_bpd_asyn_call使用，不应该被直接使用
 *
 *@参数：
 *[task]：任务对象
 *[afunc]：被调用的异步函数
 *************************************************************/
void ktask_co_asyn_call_prepare(ktask_co_t *task, ktask_co_asyn_routine_t afunc, void **pbpd);


/*********************************************************
 *@简要：
 ***异步地调用异步函数，并等待异步函数使用ktask_co_asyn_return返回
 *
 *@约定：
 ***不能使用空指针
 *
 *@参数：
 *[bp_num]：用于异步返回的，bpd断点号
 *[task]：任务对象
 *[afunc]：被调用的异步函数，异步函数应该被声明为 void func(task_t *task, event_t *ev, 参数1, 参数2..)等格式
 *[...]：被调用的异步函数的第二个之后的参数
 **********************************************************/
#define ktask_co_bpd_asyn_call(bp_num, task, afunc, ...)                                \
    do {                                                                                \
        bpd_set(bp_num);                                                                \
        ktask_co_asyn_call_prepare((task), (ktask_co_asyn_routine_t)afunc, (void **)&(bpd)); \
        afunc((task), (event_t *)NULL, ##__VA_ARGS__);                                  \
        if ((uint8_t *)(bpd) != (task->stack.cur + task->cur_ctx.stack_used)) {         \
            *(uint8_t *)(bpd) = 1;                                                      \
            return ;                                                                    \
        }                                                                               \
        (bpd) = KTASK_BPD(task);                                                        \
        bpd_restore_point(bp_num):;                                                     \
    } while (0)


#endif /* __KTASK_CO_H__ */

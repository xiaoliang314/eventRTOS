#include <os/kernel.h>
#include <drivers/regs_util.h>
#include <drivers/timer_port.h>

/* systick定时器最小定时时间（即SysTick中断延迟的最长时间，当前假设为16000个cycles） */
#define SYSTICK_MIN_TIMEOUT         (16000)

/* SYSTICK寄存器定义 */
#define SYSTICK_BASE                0xE000E010

/* SYSTICK MAX TIMEOUT */
#define SYSTICK_MAX_COUNT_CYCLES    BIT(24)

/* SYSTICK CSR寄存器及字段 */
#define SYSTICK_R_CSR               REG_ENTITY(0x0000)
#define SYSTICK_F_COUNTFLAG         REG_FIELD(0x0000, 16, 16)
#define SYSTICK_F_CLKSOURCE         REG_FIELD(0x0000, 2, 2)
#define SYSTICK_F_TICKINT           REG_FIELD(0x0000, 1, 1)
#define SYSTICK_F_ENABLE            REG_FIELD(0x0000, 0, 0)

/* SYSTICK RELOAD寄存器 */ 
#define SYSTICK_R_RELOAD            REG_ENTITY(0x0004)
#define SYSTICK_F_RELOAD            REG_FIELD(0x0004, 0, 23)

/* SYSTICK CVR寄存器 */    
#define SYSTICK_R_CVR               REG_ENTITY(0x0008)
#define SYSTICK_F_CVR               REG_FIELD(0x0008, 0, 23)

/* SYSTICK CALIB寄存器 */  
#define SYSTICK_R_CALIB             REG_ENTITY(0x000C)
#define SYSTICK_F_NOREF             REG_FIELD(0x000C, 31, 31)
#define SYSTICK_F_SKEW              REG_FIELD(0x000C, 30, 30)
#define SYSTICK_F_TENMS             REG_FIELD(0x000C, 0, 23)

/* ICSR */
#define CORTEX_M_ICSR                   0xE000ED04
#define CORTEX_SYSTICK_IRQ_PENDSET      BIT(26)

struct drv_systick_ctx_s {
    ktime_tick_t overflow;
    ktime_tick_t expiry;
};

static struct drv_systick_ctx_s drv_ctx;

void cortex_m_systick_init(void)
{
    /* 初始化Systick */
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD, SYSTICK_MAX_COUNT_CYCLES - 1);
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_CVR, 0);

    REG_WRITE_FIELDS_NO_READBACK(SYSTICK_BASE,
                                 SYSTICK_F_CLKSOURCE, 1,
                                 SYSTICK_F_TICKINT, 1,
                                 SYSTICK_F_ENABLE, 1);

    /* 因为Systick的COUNT向下计数，实际经过的时间为overflow + RELOAD - COUNT，
     * 因此，我们将RELOAD计入overflow以减少多余的运算
     */
    drv_ctx.overflow = SYSTICK_MAX_COUNT_CYCLES;
}

ktime_tick_t drv_ktime_tick_get(void)
{
    ktime_tick_t overflow;
    uint32_t cvr1, cvr2, countflag;
    uint32_t reload;

    int key = irq_lock();

    /* 以两次读取cvr的值确保countflag与cvr之间的一致性 */
    cvr1 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);
    countflag = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_F_COUNTFLAG);
    cvr2 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);

    overflow = drv_ctx.overflow;

    /* 计数器溢出检测 */
    if (cvr2 > cvr1 || countflag) {
        /* 读取CSR寄存器清除COUNTFLAG */
        (void)REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CSR);

        reload = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD);

        overflow += reload + 1;
        drv_ctx.overflow = overflow;
    }

    irq_unlock(key);

    return overflow - cvr2;
}

static void systick_reset_reload(uint32_t reload)
{
    uint32_t cvr1, cvr2, countflag;
    uint32_t old_reload;
    ktime_tick_t overflow;

    /* 保存旧的reload值用于推算当前时间 */
    old_reload = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD);
    /* 设置新的reload作为超时时间 */
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD, reload - 1);

    /* 读取先前CVR的状态，并清零开启新的reload超时 */
    cvr1 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);
    countflag = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_F_COUNTFLAG);
    cvr2 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);
    /* 设置任意值将CVR清零 */
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_CVR, cvr2);

    /* 读取CSR寄存器确保清除COUNTFLAG */
    (void)REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CSR);

    /* 推算设置reload前经过的时间 */
    overflow = drv_ctx.overflow;
    if (cvr2 > cvr1 || countflag) {
        overflow += old_reload + 1;
    }

    overflow -= cvr2;

    /* 计入新的RELOAD
     * 新的超时在CVR清零时生效，在cvr2与CVR清零之间有2个cycles的延时，将其一起计入
     */
    overflow += reload + 2;

    drv_ctx.overflow = overflow;
}

void drv_ktimer_set_expiry(ktime_tick_t expiry)
{
    int key;
    uint32_t reload;
    ktime_tick_t timeout, now;

    key = irq_lock();

    /* 无超时 */
    if (expiry == 0) {
        reload = SYSTICK_MAX_COUNT_CYCLES;
        drv_ctx.expiry = INT64_MAX;
    }
    /* 设置超时时间 */
    else {
        now = drv_ktime_tick_get();
        timeout = expiry - now;
        drv_ctx.expiry = expiry;

        /* 若超时时间小于128个cycles时间，则立即触发超时
         * 128个cycles保证实际触发时刻已经超时设置的到期值
         */
        if (timeout < 128) {
            /* Pending Systick IRQ */
            REG_WRITE_ENTITY(CORTEX_M_ICSR, CORTEX_SYSTICK_IRQ_PENDSET);
            irq_unlock(key);
            return;
        }

        if (timeout >= SYSTICK_MAX_COUNT_CYCLES * 2) {
            reload = SYSTICK_MAX_COUNT_CYCLES;
        } else if ((uint32_t)timeout > SYSTICK_MAX_COUNT_CYCLES) {
            reload = (uint32_t)timeout / 2;
        } else if ((uint32_t)timeout < SYSTICK_MIN_TIMEOUT) {
            reload = SYSTICK_MIN_TIMEOUT;
        } else {
            reload = (uint32_t)timeout;
        }
    }

    systick_reset_reload(reload);

    irq_unlock(key);
}

/* Systick中断 */
void SysTick_Handler(void)
{
    ktime_tick_t now, expiry, timeout;
    int key;

    key = irq_lock();

    /* 计算超时时间 */
    expiry = drv_ctx.expiry;
    now = drv_ktime_tick_get();
    timeout = expiry - now;

    if (timeout > 0) {
        /* 当timeout小于可systick可计数的时间时，则设置reload */
        if (timeout <= SYSTICK_MAX_COUNT_CYCLES) {
            systick_reset_reload((uint32_t)timeout);
        }

        irq_unlock(key);
        return;
    }

    irq_unlock(key);

    /* 处理超时 */
    sys_ktimer_timeout_check(now);
}

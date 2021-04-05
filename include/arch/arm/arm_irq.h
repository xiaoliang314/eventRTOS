#ifndef __ARCH_ARM_IRQ_H__
#define __ARCH_ARM_IRQ_H__

#include <bases.h>

#if defined(__ARMCC_VERSION)
#include "asm_inline_armcc.h"
#elif defined(__GNUC__)
#include "asm_inline_gcc.h"
#endif

static force_inline arch_irq_schedule_pending(void)
{
    *(volatile int *)0xE000ED04 = BIT(28);
}

#endif /* __ARCH_ARM_IRQ_H__ */

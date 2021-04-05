/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __ARCH_ASM_INLINE_ARMCC_H__
#define __ARCH_ASM_INLINE_ARMCC_H__

#include <bases.h>

static force_inline int irq_lock(void)
{
    int key;

	__asm volatile("mrs key, PRIMASK");
    __disable_irq();

    return key;
}

static force_inline void irq_unlock(int key)
{
	if (key) {
		return;
	}

    __enable_irq();
}

#endif /* __ARCH_ASM_INLINE_ARMCC_H__ */

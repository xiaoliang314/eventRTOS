/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __ARCH_ASM_INLINE_GCC_H__
#define __ARCH_ASM_INLINE_GCC_H__

#include <bases.h>

static force_inline int irq_lock(void)
{
    int key;

	__asm volatile("mrs %0, PRIMASK;"
		"cpsid i"
		: "=r" (key)
		:
		: "memory");

    return key;
}

static force_inline void irq_unlock(int key)
{
	if (key) {
		return;
	}
	__asm__ volatile(
		"cpsie i;"
		"isb"
		: : : "memory");
}

#endif /* __ARCH_ASM_INLINE_GCC_H__ */

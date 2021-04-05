/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __COMPILER_GCC_H__
#define __COMPILER_GCC_H__

#ifndef force_inline
#define force_inline    inline __attribute__((always_inline))
#endif

#ifndef __asm
#define __asm __asm__
#endif

#endif /* __COMPILER_GCC_H__ */

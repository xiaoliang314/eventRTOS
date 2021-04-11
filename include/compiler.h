/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __INCLUDE_COMPILER_H__
#define __INCLUDE_COMPILER_H__

#if defined(__ARMCC_VERSION) || defined(__GNUC__)

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#endif

#if defined(__ARMCC_VERSION)
#include <compiler/armcc.h>
#elif defined(__GNUC__)
#include <compiler/gcc.h>
#endif

#endif /* __INCLUDE_COMPILER_H__ */

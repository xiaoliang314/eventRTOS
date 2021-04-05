/*
 * Copyright (C) 2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __INCLUDE_BASES_H__
#define __INCLUDE_BASES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <compiler.h>

/* 通过类型、成员名计算成员的相对偏移 */
#ifndef offset_of
#define offset_of(type, member) \
                            (((const uint8_t *)&((type *)0)->member) - (const uint8_t *)0)
#endif

/* 通过指针、成员名计算成员的相对偏移 */
#ifndef poffset_of
#define poffset_of(eptr, member) \
                            (((const uint8_t *)&(eptr)->member) - (const uint8_t *)(eptr))
#endif

/* 通过类型、成员名、成员指针计算外部结构的地址 */
#ifndef container_of
#define container_of(mptr, type, member) \
                            ((type *)((const uint8_t *)(mptr) - (const uint8_t *)offset_of(type, member)))
#endif

/* 通过外部结构指针、成员名、成员指针计算外部结构的地址 */
#ifndef pcontainer_of
#define pcontainer_of(mptr, eptr, member) \
                            ((void *)((const uint8_t *)(mptr) - (const uint8_t *)poffset_of(eptr, member)))
#endif

/* 无符号8、16、32、64位整数最大值定义 */
#undef UINT8_MAX
#undef UINT16_MAX
#undef UINT32_MAX
#undef UINT64_MAX

#define UINT8_MAX           0xFFu
#define UINT16_MAX          0xFFFFu
#define UINT32_MAX          0xFFFFFFFFu
#define UINT64_MAX          0xFFFFFFFFFFFFFFFFull

/* 有符号8、16、32、64位整数最大值定义 */
#undef INT8_MAX
#undef INT16_MAX
#undef INT32_MAX
#undef INT64_MAX

#define INT8_MAX            0x7Fl
#define INT16_MAX           0x7FFFl
#define INT32_MAX           0x7FFFFFFFl
#define INT64_MAX           0x7FFFFFFFFFFFFFFFll

/* 有符号8、16、32、64位整数最小值定义 */
#undef INT8_MIN
#undef INT16_MIN
#undef INT32_MIN
#undef INT64_MIN

#define INT8_MIN            (-INT8_MAX - 1)
#define INT16_MIN           (-INT16_MAX - 1)
#define INT32_MIN           (-INT32_MAX - 1)
#define INT64_MIN           (-INT64_MAX - 1ll)


/* 返回两个数的最小值
 * 此宏须保证a，b在调用时保持不变
 */
#ifndef MIN
#define MIN(a, b)           ((a) > (b) ? (b) : (a))
#endif

/* 返回两个数的最大值
 * 此宏须保证a，b在调用时保持不变
 */
#ifndef MAX
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#endif

/* CPU字长大小 */
#ifndef CPU_WORD_SIZE
#define CPU_WORD_SIZE       sizeof(long)
#endif

/* 返回以N向上对齐的整数  */
#ifndef ALIGN_UP
#define ALIGN_UP(i, N)      (((((i) + ((N) - 1ul)) / (N))) * (N))

#define ALIGN2_UP(i)        ALIGN_UP(i, 2ul)
#define ALIGN4_UP(i)        ALIGN_UP(i, 4ul)
#define ALIGN8_UP(i)        ALIGN_UP(i, 8ul)
#define ALIGN_CPU_UP(i)     ALIGN_UP(i, CPU_WORD_SIZE)

#endif

/* 返回以N向下对齐的整数  */
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(i, N)    (((i) / (N)) * (N))

#define ALIGN2_DOWN(i)      ALIGN_DOWN(i, 2ul)
#define ALIGN4_DOWN(i)      ALIGN_DOWN(i, 4ul)
#define ALIGN8_DOWN(i)      ALIGN_DOWN(i, 8ul)
#define ALIGN_CPU_DOWN(i)   ALIGN_DOWN(i, CPU_WORD_SIZE)

#endif

/* 返回数组的长度 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)     (sizeof(*(arr)) ? (sizeof(arr) / sizeof(*(arr))) : 0)
#endif

/* 空指针 */
#ifndef NULL
#define NULL 0
#endif

/* 获取第N个位 */
#ifndef BIT
#define BIT(n)      (1UL << (n))
#endif

#endif /* __INCLUDE_BASES_H__ */

/*
 * Copyright (C) 2019-2021 xiaoliang<1296283984@qq.com>.
 */

#ifndef __DRIVERS_REGS_UTIL_H_
#define __DRIVERS_REGS_UTIL_H_

/**********************************************************
 * 这是一组寄存器操作辅助宏接口，
 * 它允许你将寄存器的位字段友好的定义出来，并进行高效的读写
 * 位字段的读写可以很容易被编译识别并优化，在提高性能的同时降低代码的大小
 * 字面常量的多字段写入可以被编译器计算并合并为一个值
 **********************************************************/

/* 定义寄存器字段范围
 * 可以使用标识符很方便表示一个字段，并使用接口进行读写
 * 示例：
 *     #define GPIO_F_PA0_OUT   REG_FIELD(0x0010, 0, 0)
 *     #define GPIO_F_PA2_IN    REG_FIELD(0x0010, 10, 10)
 */
#define REG_FIELD(offset, low_bit, high_bit)            (offset), (low_bit), (high_bit)

/* 定义寄存器
 * 可以使用标识符很方便表示一个寄存器，并使用接口进行读写
 * 示例：
 *     #define GPIO_R_PA_INOUT  REG_ENTITY(0x0010)
 *     #define GPIO_R_PB_INOUT  REG_ENTITY(0x0020)
 */
#define REG_ENTITY(offset)                              REG_FIELD(offset, 0, 31)

/* 写多个寄存器字段，字段必须是同一个寄存器
 * 该寄存器下未指明的其他字段将被回读并保持不变
 * 注意：该操作将会对寄存器产生读操作，对某些寄存器将会产生副作用，如：UART的DR寄存器
 * 示例：
 * REG_WRITE_FIELDS(GPIO_BASE, 
 *                  GPIO_F_PA0_OUT, 1,
 *                  GPIO_F_PA2_OUT, 0,
 *                  GPIO_F_PA3_OUT, 1);
 */
#define REG_WRITE_FIELDS(DEV_BASE, ...)                 REG_WRITE_FIELD3(DEV_BASE, REG_FIELDS_OFFSET(__VA_ARGS__), REG_FIELDS_MASK(__VA_ARGS__), REG_FIELDS_VAL(__VA_ARGS__))

/* 写字段无回读，字段必须是同一个寄存器
 * 这是最高效的一种多字段读写方式，该寄存器下未指明的其他字段将被设置为0
 * 示例：
 * REG_WRITE_FIELDS_NO_READBACK(GPIO_BASE, 
 *                              GPIO_F_PA0_SET, 1,
 *                              GPIO_F_PA2_CLR, 1,
 *                              GPIO_F_PA3_SET, 1);
 */
#define REG_WRITE_FIELDS_NO_READBACK(DEV_BASE, ...)     REG_WRITE_FIELD3_NO_READBACK(DEV_BASE, REG_FIELDS_OFFSET(__VA_ARGS__), REG_FIELDS_VAL(__VA_ARGS__))

/* 构造指定字段值
 * 可以使用该接口构造同一个寄存器多个字段组合值
 * 示例：
 * int cfg1 = REG_FIELDS_VAL(GPIO_F_PA0_SET, 1,
 *                          GPIO_F_PA2_CLR, 1,
 *                          GPIO_F_PA3_SET, 1);
 * 
 * int cfg2 = REG_FIELDS_VAL(GPIO_F_PA0_CLR, 1,
 *                          GPIO_F_PA2_SET, 1,
 *                          GPIO_F_PA3_CLR, 1);
 * 
 * REG_WRITE_FIELD(GPIO_BASE, GPIO_R_PA_BSR, cfg1);
 * REG_WRITE_FIELD(GPIO_BASE, GPIO_R_PA_BSR, cfg2);
 */
#define REG_FIELDS_VAL(...)                             __REG_VA_NARGS_EXPAND(__REG_MACRO_CAT_2(__REG_FIELDS_VAL_, REG_VA_NARGS(__VA_ARGS__))(__VA_ARGS__))

/* 读字段
 * 读取一个字段的值，值的范围为2^N - 1，N为字段的宽度
 * 示例：
 * int pa0_val = REG_READ_FIELD(GPIO_BASE, GPIO_F_PA0_IN);
 */
#define REG_READ_FIELD(DEV_BASE, FIELD)		            REG_READ_FIELD2(DEV_BASE, FIELD)

/* 从值中读字段内容
 * 读取一个字段的值，值的范围为2^N - 1，N为字段的宽度
 * 示例：
 * int pa_val = REG_READ_FIELD(GPIO_BASE, GPIO_R_PA_IN)
 * int pa0_val = REG_VAL_READ_FIELD(pa_val, GPIO_F_PA0_IN);
 */
#define REG_VAL_READ_FIELD(VAL, FIELD)                  REG_VAL_READ_FIELD2(VAL, FIELD)

/* 使用地址读取寄存器值
 * 示例：
 * int val = REG_READ_ENTIRY(0xE000E040);
 */
#define REG_READ_ENTIRY(addr)                           (REG_ENTITY_VAL(addr, 0))

/* 字段范围读
 * 示例：
 * int field_val = REG_READ_FIELD_RANGE(0xE000E040, 12, 14)
 */
#define REG_READ_FIELD_RANGE(addr, low, high)           REG_READ_FIELD2(addr, 0, low, high)

/* 使用带回读的方式写一个字段
 * 示例：
 * REG_WRITE_FIELD(GPIO_BASE, GPIO_F_PA0_OUT, 1)
 */
#define REG_WRITE_FIELD(DEV_BASE, FIELD, VAL)           REG_WRITE_FIELD2(DEV_BASE, FIELD, VAL)

/* 写寄存器
 * 示例：
 * REG_READ_ENTIRY(0xE000E040, val);
 */
#define REG_WRITE_ENTITY(addr, val)                     (REG_ENTITY_VAL(addr, 0) = (val))

/* 字段范围写
 * 示例：
 * REG_WRITE_FIELD_RANGE(0xE000E040, 12, 14, 6)
 */
#define REG_WRITE_FIELD_RANGE(addr, low, high, val)     REG_WRITE_FIELD2(addr, 0, low, high, val)

/* 写字段所在的整个寄存器值
 * 示例：
 * REG_WRITE_FIELD_REG(GPIO_BASE, GPIO_F_PA0_OUT, 0xFFFF);
 */
#define REG_WRITE_FIELD_REG(base, field, val)           REG_WRITE_FIELD2(base, REG_FIELD_OFFSET2(field), 0, 31, val)

#ifndef BIT
#define BIT(n)   (1 << n)
#endif

/********************************* 详细实现 ********************************/

#define REG_VA_EXPAND_ARGS(...)  __VA_ARGS__

#define REG_VA_NARGS_EXPAND(x)  x

#define REG_VA_NARGS(...)  REG_VA_NARGS_EXPAND(__REG_VA_NARGS(0, ##__VA_ARGS__,\
                                    128, \
                                    127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, \
                                    111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, \
                                    95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
                                    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, \
                                    63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, \
                                    47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, \
                                    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
                                    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define __REG_VA_NARGS(  _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, \
                    _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, \
                    _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, \
                    _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, \
                    _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, _78, _79, \
                    _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, \
                    _96, _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, \
                    _112, _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125, _126, _127, \
                    _128, N, ...) N

#define __REG_FIELDS_MASK__(offset, low, high, val)          (((1UL << ((high) - (low) + 1)) - 1UL) << (low))
#define __REG_FIELDS_MASK_4(offset, low, high, val)           __REG_FIELDS_MASK__(offset, low, high, val)
#define __REG_FIELDS_MASK_8(offset, low, high, val, ...)      (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_4(__VA_ARGS__))
#define __REG_FIELDS_MASK_12(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_8(__VA_ARGS__))
#define __REG_FIELDS_MASK_16(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_12(__VA_ARGS__))
#define __REG_FIELDS_MASK_20(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_16(__VA_ARGS__))
#define __REG_FIELDS_MASK_24(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_20(__VA_ARGS__))
#define __REG_FIELDS_MASK_28(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_24(__VA_ARGS__))
#define __REG_FIELDS_MASK_32(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_28(__VA_ARGS__))
#define __REG_FIELDS_MASK_36(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_32(__VA_ARGS__))
#define __REG_FIELDS_MASK_40(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_36(__VA_ARGS__))
#define __REG_FIELDS_MASK_44(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_40(__VA_ARGS__))
#define __REG_FIELDS_MASK_48(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_44(__VA_ARGS__))
#define __REG_FIELDS_MASK_52(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_48(__VA_ARGS__))
#define __REG_FIELDS_MASK_56(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_52(__VA_ARGS__))
#define __REG_FIELDS_MASK_60(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_56(__VA_ARGS__))
#define __REG_FIELDS_MASK_64(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_60(__VA_ARGS__))
#define __REG_FIELDS_MASK_68(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_64(__VA_ARGS__))
#define __REG_FIELDS_MASK_72(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_68(__VA_ARGS__))
#define __REG_FIELDS_MASK_76(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_72(__VA_ARGS__))
#define __REG_FIELDS_MASK_80(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_76(__VA_ARGS__))
#define __REG_FIELDS_MASK_84(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_80(__VA_ARGS__))
#define __REG_FIELDS_MASK_88(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_84(__VA_ARGS__))
#define __REG_FIELDS_MASK_92(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_88(__VA_ARGS__))
#define __REG_FIELDS_MASK_96(offset, low, high, val, ...)     (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_92(__VA_ARGS__))
#define __REG_FIELDS_MASK_100(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_96(__VA_ARGS__))
#define __REG_FIELDS_MASK_104(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_100(__VA_ARGS__))
#define __REG_FIELDS_MASK_108(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_104(__VA_ARGS__))
#define __REG_FIELDS_MASK_112(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_108(__VA_ARGS__))
#define __REG_FIELDS_MASK_116(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_112(__VA_ARGS__))
#define __REG_FIELDS_MASK_120(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_116(__VA_ARGS__))
#define __REG_FIELDS_MASK_124(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_120(__VA_ARGS__))
#define __REG_FIELDS_MASK_128(offset, low, high, val, ...)    (__REG_FIELDS_MASK__(offset, low, high, val) | __REG_FIELDS_MASK_124(__VA_ARGS__))

#define __REG_FIELDS_VAL__(offset, low, high, val)           ((((1UL << ((high) - (low) + 1)) - 1UL) & (val)) << (low))
#define __REG_FIELDS_VAL_4(offset, low, high, val)           __REG_FIELDS_VAL__(offset, low, high, val)
#define __REG_FIELDS_VAL_8(offset, low, high, val, ...)      (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_4(__VA_ARGS__))
#define __REG_FIELDS_VAL_12(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_8(__VA_ARGS__))
#define __REG_FIELDS_VAL_16(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_12(__VA_ARGS__))
#define __REG_FIELDS_VAL_20(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_16(__VA_ARGS__))
#define __REG_FIELDS_VAL_24(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_20(__VA_ARGS__))
#define __REG_FIELDS_VAL_28(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_24(__VA_ARGS__))
#define __REG_FIELDS_VAL_32(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_28(__VA_ARGS__))
#define __REG_FIELDS_VAL_36(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_32(__VA_ARGS__))
#define __REG_FIELDS_VAL_40(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_36(__VA_ARGS__))
#define __REG_FIELDS_VAL_44(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_40(__VA_ARGS__))
#define __REG_FIELDS_VAL_48(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_44(__VA_ARGS__))
#define __REG_FIELDS_VAL_52(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_48(__VA_ARGS__))
#define __REG_FIELDS_VAL_56(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_52(__VA_ARGS__))
#define __REG_FIELDS_VAL_60(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_56(__VA_ARGS__))
#define __REG_FIELDS_VAL_64(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_60(__VA_ARGS__))
#define __REG_FIELDS_VAL_68(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_64(__VA_ARGS__))
#define __REG_FIELDS_VAL_72(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_68(__VA_ARGS__))
#define __REG_FIELDS_VAL_76(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_72(__VA_ARGS__))
#define __REG_FIELDS_VAL_80(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_76(__VA_ARGS__))
#define __REG_FIELDS_VAL_84(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_80(__VA_ARGS__))
#define __REG_FIELDS_VAL_88(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_84(__VA_ARGS__))
#define __REG_FIELDS_VAL_92(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_88(__VA_ARGS__))
#define __REG_FIELDS_VAL_96(offset, low, high, val, ...)     (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_92(__VA_ARGS__))
#define __REG_FIELDS_VAL_100(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_96(__VA_ARGS__))
#define __REG_FIELDS_VAL_104(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_100(__VA_ARGS__))
#define __REG_FIELDS_VAL_108(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_104(__VA_ARGS__))
#define __REG_FIELDS_VAL_112(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_108(__VA_ARGS__))
#define __REG_FIELDS_VAL_116(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_112(__VA_ARGS__))
#define __REG_FIELDS_VAL_120(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_116(__VA_ARGS__))
#define __REG_FIELDS_VAL_124(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_120(__VA_ARGS__))
#define __REG_FIELDS_VAL_128(offset, low, high, val, ...)    (__REG_FIELDS_VAL__(offset, low, high, val) | __REG_FIELDS_VAL_124(__VA_ARGS__))

#define __REG_VA_NARGS_EXPAND(...)  __REG_EVAL(__VA_ARGS__)

#define __REG_EVAL(...) __REG_EVAL1(__REG_EVAL1(__REG_EVAL1(__VA_ARGS__)))
#define __REG_EVAL1(...) __VA_ARGS__

#define __REG_MACRO_CAT_2(x, y)  ____REG_MACRO_CAT_2(x, y)
#define ____REG_MACRO_CAT_2(x, y) x##y

#define REG_FIELDS_OFFSET(...)                           __REG_VA_NARGS_EXPAND(REG_FIELD_OFFSET2(__VA_ARGS__))

#define REG_ENTITY_VAL(base, offset)                        *(volatile unsigned int *)((unsigned int)(base) + (unsigned int)(offset))

#define REG_FIELDS_MASK(...)                             __REG_VA_NARGS_EXPAND(__REG_MACRO_CAT_2(__REG_FIELDS_MASK_, REG_VA_NARGS(__VA_ARGS__))(__VA_ARGS__))

#define REG_FIELD_OFFSET2(offset, low, high, ...)        (offset)
#define REG_FIELD_OFFSET(FIELD)                          REG_FIELD_OFFSET2(FIELD)

#define REG_FIELD_LOW_BIT2(offset, low, high)            (low)
#define REG_FIELD_LOW_BIT(FIELD)                         REG_FIELD_LOW_BIT2(FIELD)

#define REG_FIELD_HIGH_BIT2(offset, low, high)           (high)
#define REG_FIELD_HIGH_BIT(FIELD)                        REG_FIELD_HIGH_BIT2(FIELD)

#define REG_FIELD_FIELD2(offset, low_bit, high_bit, field_low_bit, field_high_bit)   \
                (offset), ((low_bit) + (field_low_bit)), ((low_bit) + (field_high_bit))
#define REG_FIELD_FIELD(FIELD, low, high)                REG_FIELD_FIELD2(FIELD, low, high)

#define REG_FIELD_RANGE_MASK(low_bit, high_bit)	       \
    ((1UL << ((high_bit) - (low_bit) + 1UL)) - 1UL)

#define REG_READ_FIELD2(base, offset, low, high)        \
    (((high) - (low) >= 31) ? REG_ENTITY_VAL(base, offset) : ((REG_ENTITY_VAL(base, offset) >> (low)) & REG_FIELD_RANGE_MASK((low), (high))))

#define REG_VAL_READ_FIELD2(val, offset, low, high)        \
    (((high) - (low) >= 31) ? (val) : (((val) >> (low)) & REG_FIELD_RANGE_MASK((low), (high))))

#define REG_WRITE_FIELD2(base, offset, low, high, val)  \
	(REG_ENTITY_VAL(base, offset) = ((high) - (low) >= 31) ? (val) : ((REG_ENTITY_VAL(base, offset) & ~(REG_FIELD_RANGE_MASK(low, high) << (low))) | (((val) & REG_FIELD_RANGE_MASK(low, high)) << (low))))

#define REG_WRITE_FIELD2_R(base, offset, low, high, val)  \
    (REG_ENTITY_VAL(base, offset) = (val))

#define REG_WRITE_FIELD3(base, offset, mask, val)       \
    (REG_ENTITY_VAL(base, offset) = ((mask) == 0xFFFFFFFFUL ? (val) : ((REG_ENTITY_VAL(base, offset) & ~(mask)) | (val))))

#define REG_WRITE_FIELD3_NO_READBACK(base, offset, val) \
    (REG_ENTITY_VAL(base, offset) = (val))

#endif /* __DRIVERS_REGS_UTIL_H_ */

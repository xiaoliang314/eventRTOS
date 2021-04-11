#include <os/kernel.h>
#include <drivers/cortex_m/systick.h>
#include <drivers/regs_util.h>
#include "stm32f10x.h"

/* RCC外设复位寄存器字段定义 */
#define RCC_R_APB2RSTR              REG_ENTITY(0x000C)
#define RCC_F_APB2RSTR_AFIOEN       REG_FIELD(0x000C, 0, 0)
#define RCC_F_APB2RSTR_IOPAEN       REG_FIELD(0x000C, 2, 2)
#define RCC_F_APB2RSTR_IOPBEN       REG_FIELD(0x000C, 3, 3)
#define RCC_F_APB2RSTR_IOPCEN       REG_FIELD(0x000C, 4, 4)
#define RCC_F_APB2RSTR_IOPDEN       REG_FIELD(0x000C, 5, 5)

/* RCC外设使能寄存器字段定义 */
#define RCC_R_APB2ENR               REG_ENTITY(0x0018)
#define RCC_F_APB2ENR_AFIOEN        REG_FIELD(0x0018, 0, 0)
#define RCC_F_APB2ENR_IOPAEN        REG_FIELD(0x0018, 2, 2)
#define RCC_F_APB2ENR_IOPBEN        REG_FIELD(0x0018, 3, 3)
#define RCC_F_APB2ENR_IOPCEN        REG_FIELD(0x0018, 4, 4)
#define RCC_F_APB2ENR_IOPDEN        REG_FIELD(0x0018, 5, 5)

/* GPIO模式字段定义 */
#define GPIO_F_PIN0_MODE            REG_FIELD(0x0000,  0, 1)
#define GPIO_F_PIN1_MODE            REG_FIELD(0x0000,  4, 5)
#define GPIO_F_PIN2_MODE            REG_FIELD(0x0000,  8, 9)
#define GPIO_F_PIN3_MODE            REG_FIELD(0x0000, 12, 13)
#define GPIO_F_PIN4_MODE            REG_FIELD(0x0000, 16, 17)
#define GPIO_F_PIN5_MODE            REG_FIELD(0x0000, 20, 21)
#define GPIO_F_PIN6_MODE            REG_FIELD(0x0000, 24, 25)
#define GPIO_F_PIN7_MODE            REG_FIELD(0x0000, 28, 29)

#define GPIO_F_PIN8_MODE            REG_FIELD(0x0004,  0, 1)
#define GPIO_F_PIN9_MODE            REG_FIELD(0x0004,  4, 5)
#define GPIO_F_PIN10_MODE           REG_FIELD(0x0004,  8, 9)
#define GPIO_F_PIN11_MODE           REG_FIELD(0x0004, 12, 13)
#define GPIO_F_PIN12_MODE           REG_FIELD(0x0004, 16, 17)
#define GPIO_F_PIN13_MODE           REG_FIELD(0x0004, 20, 21)
#define GPIO_F_PIN14_MODE           REG_FIELD(0x0004, 24, 25)
#define GPIO_F_PIN15_MODE           REG_FIELD(0x0004, 28, 29)

#define GPIO_PIN_MODE_IN            0
#define GPIO_PIN_MODE_OUT_10MHZ     1
#define GPIO_PIN_MODE_OUT_2MHZ      2
#define GPIO_PIN_MODE_OUT_50MHZ     3

/* GPIO配置字段定义 */
#define GPIO_F_PIN0_CFG             REG_FIELD(0x0000,  2, 3)
#define GPIO_F_PIN1_CFG             REG_FIELD(0x0000,  6, 7)
#define GPIO_F_PIN2_CFG             REG_FIELD(0x0000, 10, 11)
#define GPIO_F_PIN3_CFG             REG_FIELD(0x0000, 14, 15)
#define GPIO_F_PIN4_CFG             REG_FIELD(0x0000, 18, 19)
#define GPIO_F_PIN5_CFG             REG_FIELD(0x0000, 22, 23)
#define GPIO_F_PIN6_CFG             REG_FIELD(0x0000, 26, 27)
#define GPIO_F_PIN7_CFG             REG_FIELD(0x0000, 30, 31)

#define GPIO_F_PIN8_CFG             REG_FIELD(0x0004,  2, 3)
#define GPIO_F_PIN9_CFG             REG_FIELD(0x0004,  6, 7)
#define GPIO_F_PIN10_CFG            REG_FIELD(0x0004, 10, 11)
#define GPIO_F_PIN11_CFG            REG_FIELD(0x0004, 14, 15)
#define GPIO_F_PIN12_CFG            REG_FIELD(0x0004, 18, 19)
#define GPIO_F_PIN13_CFG            REG_FIELD(0x0004, 22, 23)
#define GPIO_F_PIN14_CFG            REG_FIELD(0x0004, 26, 27)
#define GPIO_F_PIN15_CFG            REG_FIELD(0x0004, 30, 31)

#define GPIO_PIN_CFG_IN_ANA         0
#define GPIO_PIN_CFG_IN_NOPULL      1
#define GPIO_PIN_CFG_IN_PULL        2

#define GPIO_PIN_CFG_OUT_PULL       0
#define GPIO_PIN_CFG_OUT_NOPULL     1
#define GPIO_PIN_CFG_OUT_AF_PULL    2
#define GPIO_PIN_CFG_OUT_AF_NOPULL  3

/* GPIO输入字段定义 */
#define GPIO_F_PIN0_IN              REG_FIELD(0x0008,  0, 0)
#define GPIO_F_PIN1_IN              REG_FIELD(0x0008,  1, 1)
#define GPIO_F_PIN2_IN              REG_FIELD(0x0008,  2, 2)
#define GPIO_F_PIN3_IN              REG_FIELD(0x0008,  3, 3)
#define GPIO_F_PIN4_IN              REG_FIELD(0x0008,  4, 4)
#define GPIO_F_PIN5_IN              REG_FIELD(0x0008,  5, 5)
#define GPIO_F_PIN6_IN              REG_FIELD(0x0008,  6, 6)
#define GPIO_F_PIN7_IN              REG_FIELD(0x0008,  7, 7)
#define GPIO_F_PIN8_IN              REG_FIELD(0x0008,  8, 8)
#define GPIO_F_PIN9_IN              REG_FIELD(0x0008,  9, 9)
#define GPIO_F_PIN10_IN             REG_FIELD(0x0008, 10, 10)
#define GPIO_F_PIN11_IN             REG_FIELD(0x0008, 11, 11)
#define GPIO_F_PIN12_IN             REG_FIELD(0x0008, 12, 12)
#define GPIO_F_PIN13_IN             REG_FIELD(0x0008, 13, 13)
#define GPIO_F_PIN14_IN             REG_FIELD(0x0008, 14, 14)
#define GPIO_F_PIN15_IN             REG_FIELD(0x0008, 15, 15)

/* GPIO输出字段定义 */
#define GPIO_F_PIN0_OUT              REG_FIELD(0x000C,  0, 0)
#define GPIO_F_PIN1_OUT              REG_FIELD(0x000C,  1, 1)
#define GPIO_F_PIN2_OUT              REG_FIELD(0x000C,  2, 2)
#define GPIO_F_PIN3_OUT              REG_FIELD(0x000C,  3, 3)
#define GPIO_F_PIN4_OUT              REG_FIELD(0x000C,  4, 4)
#define GPIO_F_PIN5_OUT              REG_FIELD(0x000C,  5, 5)
#define GPIO_F_PIN6_OUT              REG_FIELD(0x000C,  6, 6)
#define GPIO_F_PIN7_OUT              REG_FIELD(0x000C,  7, 7)
#define GPIO_F_PIN8_OUT              REG_FIELD(0x000C,  8, 8)
#define GPIO_F_PIN9_OUT              REG_FIELD(0x000C,  9, 9)
#define GPIO_F_PIN10_OUT             REG_FIELD(0x000C, 10, 10)
#define GPIO_F_PIN11_OUT             REG_FIELD(0x000C, 11, 11)
#define GPIO_F_PIN12_OUT             REG_FIELD(0x000C, 12, 12)
#define GPIO_F_PIN13_OUT             REG_FIELD(0x000C, 13, 13)
#define GPIO_F_PIN14_OUT             REG_FIELD(0x000C, 14, 14)
#define GPIO_F_PIN15_OUT             REG_FIELD(0x000C, 15, 15)

/* GPIO位设置/清除字段定义 */
#define GPIO_F_PIN0_SET              REG_FIELD(0x0010,  0, 0)
#define GPIO_F_PIN1_SET              REG_FIELD(0x0010,  1, 1)
#define GPIO_F_PIN2_SET              REG_FIELD(0x0010,  2, 2)
#define GPIO_F_PIN3_SET              REG_FIELD(0x0010,  3, 3)
#define GPIO_F_PIN4_SET              REG_FIELD(0x0010,  4, 4)
#define GPIO_F_PIN5_SET              REG_FIELD(0x0010,  5, 5)
#define GPIO_F_PIN6_SET              REG_FIELD(0x0010,  6, 6)
#define GPIO_F_PIN7_SET              REG_FIELD(0x0010,  7, 7)
#define GPIO_F_PIN8_SET              REG_FIELD(0x0010,  8, 8)
#define GPIO_F_PIN9_SET              REG_FIELD(0x0010,  9, 9)
#define GPIO_F_PIN10_SET             REG_FIELD(0x0010, 10, 10)
#define GPIO_F_PIN11_SET             REG_FIELD(0x0010, 11, 11)
#define GPIO_F_PIN12_SET             REG_FIELD(0x0010, 12, 12)
#define GPIO_F_PIN13_SET             REG_FIELD(0x0010, 13, 13)
#define GPIO_F_PIN14_SET             REG_FIELD(0x0010, 14, 14)
#define GPIO_F_PIN15_SET             REG_FIELD(0x0010, 15, 15)

#define GPIO_F_PIN0_CLR              REG_FIELD(0x0010, 16, 16)
#define GPIO_F_PIN1_CLR              REG_FIELD(0x0010, 17, 17)
#define GPIO_F_PIN2_CLR              REG_FIELD(0x0010, 18, 18)
#define GPIO_F_PIN3_CLR              REG_FIELD(0x0010, 19, 19)
#define GPIO_F_PIN4_CLR              REG_FIELD(0x0010, 20, 20)
#define GPIO_F_PIN5_CLR              REG_FIELD(0x0010, 21, 21)
#define GPIO_F_PIN6_CLR              REG_FIELD(0x0010, 22, 22)
#define GPIO_F_PIN7_CLR              REG_FIELD(0x0010, 23, 23)
#define GPIO_F_PIN8_CLR              REG_FIELD(0x0010, 24, 24)
#define GPIO_F_PIN9_CLR              REG_FIELD(0x0010, 25, 25)
#define GPIO_F_PIN10_CLR             REG_FIELD(0x0010, 26, 26)
#define GPIO_F_PIN11_CLR             REG_FIELD(0x0010, 27, 27)
#define GPIO_F_PIN12_CLR             REG_FIELD(0x0010, 28, 28)
#define GPIO_F_PIN13_CLR             REG_FIELD(0x0010, 29, 29)
#define GPIO_F_PIN14_CLR             REG_FIELD(0x0010, 30, 30)
#define GPIO_F_PIN15_CLR             REG_FIELD(0x0010, 31, 31)

#define LED0_OUT                     GPIO_F_PIN2_OUT
#define LED1_OUT                     GPIO_F_PIN8_OUT

/* LED闪烁定时器：频率10Hz */
static void on_led1_timer(void *cb_data, kevent_t *e)
{
    ktimer_event_t *timer = (ktimer_event_t*)e;
    e->bp++;

    /* 设置LED */
    REG_WRITE_FIELD(GPIOA_BASE, LED1_OUT, (e->bp & 1));
    ktimer_start_expiry(timer, ktimer_expiry_get(timer) + ktime_ms_to_tick(1000));
}

/* LED闪烁定时器：频率2Hz */
static void on_led0_timer(void *cb_data, kevent_t *e)
{
    ktimer_event_t *timer = (ktimer_event_t*)e;
    uint8_t *bpd = &e->bp;

    bpd_begin(2);

    while (1) {
        /* 打开LED */
        REG_WRITE_FIELD(GPIOD_BASE, LED0_OUT, 0);

        ktimer_start_expiry(timer, ktimer_expiry_get(timer) + ktime_us_to_tick(12500));
        bpd_yield(1);

        /* 关闭LED */
        REG_WRITE_FIELD(GPIOD_BASE, LED0_OUT, 1);

        ktimer_start_expiry(timer, ktimer_expiry_get(timer) + ktime_us_to_tick(12500));
        bpd_yield(2);
    }

    bpd_end();
}

/* 定义两个定时器 */
static ktimer_event_t led0_timer = KTIMER_EVENT_STATIC_INIT(led0_timer, on_led0_timer, 0, KEVENT_PRIORITY_LOWER_GROUP);
static ktimer_event_t led1_timer = KTIMER_EVENT_STATIC_INIT(led1_timer, on_led1_timer, 0, KEVENT_PRIORITY_MIDDLE_GROUP);

int main()
{
    NVIC_SetPriority(PendSV_IRQn, 15);
    /* 初始化systick驱动 */
    cortex_m_systick_init();

    /* 打开GPIO ABCD门控 */
    REG_WRITE_FIELDS(RCC_BASE, RCC_F_APB2ENR_IOPAEN, 1,
                               RCC_F_APB2ENR_IOPDEN, 1);

    /* 配置为输出推免模式 */
    REG_WRITE_FIELDS(GPIOD_BASE, 
                            GPIO_F_PIN2_MODE, GPIO_PIN_MODE_OUT_50MHZ,
                            GPIO_F_PIN2_CFG,  GPIO_PIN_CFG_OUT_PULL);

    REG_WRITE_FIELDS(GPIOA_BASE, 
                            GPIO_F_PIN8_MODE, GPIO_PIN_MODE_OUT_50MHZ,
                            GPIO_F_PIN8_CFG,  GPIO_PIN_CFG_OUT_PULL);

    /* 启动LED闪烁定时器 */
    ktimer_start_ms(&led1_timer, 100);
    ktimer_start_ms(&led0_timer, 106);

    while (1);
}

#include <os/time.h>
#include "system_stm32f10x.h"

ktime_ms_t drv_ktime_tick_to_ms(ktime_tick_t tick)
{
    return tick / (SystemCoreClock / 1000);
}

ktime_us_t drv_ktime_tick_to_us(ktime_tick_t tick)
{
    return tick / (SystemCoreClock / 1000000);
}

ktime_tick_t drv_ktime_us_to_tick(ktime_us_t us)
{
    return us * (SystemCoreClock / 1000000);
}

ktime_tick_t drv_ktime_ms_to_tick(ktime_ms_t ms)
{
    return ms * (SystemCoreClock / 1000);
}

/*
File Name    : yc_os_middle.c
Author       : Yichip
Version      : V1.0
Date         : 2021/7/15
Description  : ucos middle layer encapsulation.
*/

#if defined ( __CC_ARM )

__asm void PENDSV_IRQHandler(void)
{
    extern OS_CPU_PendSVHandler;
    PRESERVE8
    
    ldr r0,=OS_CPU_PendSVHandler
	bx r0
}

__asm void SYSTICK_IRQHandler(void)
{
    extern OS_CPU_SysTickHandler;
    PRESERVE8
    
    ldr r0, = OS_CPU_SysTickHandler
	bx r0
}

#elif defined ( __GNUC__ )

void PENDSV_IRQHandler(void)
{
    __asm volatile(
    ldr r0, = OS_CPU_PendSVHandler
	bx r0
    );
}

void SYSTICK_IRQHandler(void)
{
    __asm volatile(
    ldr r0, = OS_CPU_SysTickHandler
	bx r0
    );
}

#elif defined ( __ICCARM__ )

__asm void PENDSV_IRQHandler(void)
{
    asm("ldr r0, = OS_CPU_PendSVHandler");
    asm("bx r0");
}

__asm void SYSTICK_IRQHandler(void)
{
    asm("ldr r0, = OS_CPU_SysTickHandler");
    asm("bx r0");
}

#endif
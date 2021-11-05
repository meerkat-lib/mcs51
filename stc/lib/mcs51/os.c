#include "os.h"

u8 task_id = 0;
u8 tasks_delay[OS_TASKS];
u8 tasks_status = 0xff; // status 0:suspend, 1:ready
u8 tasks_stack[OS_TASKS][TASK_STACK_SIZE];
u8 tasks_sp[OS_TASKS];
u8 task_idle_stack[16];

u8 const BIT_MASKS[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

void task_switch(void)
{
    if (task_id < OS_TASKS)
    {
        tasks_sp[task_id] = SP;
    }
    u8 id = 0;
    while (id < OS_TASKS)
    {
        if ((tasks_status & BIT_MASKS[id]) != 0)
        {
            task_id = id;
            task_start(id);
            return;
        }
        id++;
    }
    task_id = OS_TASKS;
    enter_idle_mode();
    exit_critical();
}

void task_idle(void)
{
    while (1)
    {
        idle();
    }    
}

void os_start(void)
{
    enter_critical();

    //初始化定时器
    /*
    EA = 1;
    IT0 = 1;
    TMOD = 0x01;
    ET0 = 1;  
    TH0 = (65536 - UPDATE_TIME)/256;
    TL0 = (65536 - UPDATE_TIME)%256;*/

    task_idle_stack[0] = (u16)task_idle & 0xff;
    task_idle_stack[1] = (u16)task_idle >> 8;

    u8 i = 0;
    while (i < OS_TASKS)
    {
        tasks_delay[i] = 0;
        tasks_sp[i] = (u8)tasks_stack[i] + 1;
        tasks_stack[i][0] = (u16)tasks[i] & 0xff;
        tasks_stack[i][1] = (u16)tasks[i] >> 8;
        i++;
    }
    
    task_start(0);
}

void os_tick(void) __interrupt(OS_TIMER_ISR)
{
    u8 i = 0;
    while (i < OS_TASKS)
    {
        if (tasks_delay[i] != 0)
        {
            tasks_delay[i]--;
            if (tasks_delay[i] == 0)
            {
                task_ready(i);
            }
        }
        i++;
    }
    enter_critical();
    task_id = OS_TASKS;
    task_switch();
}
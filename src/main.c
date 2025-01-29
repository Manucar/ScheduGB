#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdio.h>
#include <string.h>

#include "scheduler.h"

void task0(void)
{       
    while (1)
    {
        // printf function is non-reentrant in gbdk-2020, mark it as critical
        // section in order to be executed correctly.
        CRITICAL
        {
            for (int i = 0; i < 10; i++)
            {
                printf("%u\n",i);
            }            
        }
        scheduler_yield();
    }
}

void task1(void)
{
    while (1)
    {
        CRITICAL
        {
            printf("%u\n",41);
        }
        scheduler_yield();
    }
}

void task2(void)
{
    while (1)
    {
        CRITICAL
        {
            printf("%u\n",42);
        }
        scheduler_yield();
    }
}

void main(void)
{
    scheduler_start();
}

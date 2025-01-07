#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdio.h>
#include <string.h>

#include "scheduler.h"

void task0(void)
{       
    while (1)
    {
        printf("%u\n",40);
    }
}

void task1(void)
{
    while (1)
    {
        printf("%u\n",41);
    }
}

void task2(void)
{
    while (1)
    {
        printf("%u\n",42);
    }
}

void main(void)
{
    scheduler_start();
}

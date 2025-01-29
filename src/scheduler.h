#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <stdint.h>

#define NUM_OF_TASKS    3       // Number of static task allocated
#define CPU_REGS        6       // Number of CPU register available
#define PC_POS          2       // PC position into the stack frame

// List of task prototypes
void task0(void);
void task1(void);
void task2(void);

// Define stack sizes for each task
#define TASK0_STACKSIZE 100
#define TASK1_STACKSIZE 200
#define TASK2_STACKSIZE 250

// Define the SYS_CFG_TSK_TABLE macro to create task entries
#define SYS_CFG_TSK_TABLE(S) \
    S(0,    task0,  ,   TASK0_STACKSIZE) \
    S(1,    task1,  ,   TASK1_STACKSIZE) \
    S(2,    task2,  ,   TASK2_STACKSIZE)

// Define helper macros for task memory and task table entries
#define STATIC_MEMORY(id, function, attr, stacksize) \
    static uint16_t sys_tsk_##id##_sMemory[stacksize];

#define STATIC_TASK_TABLE(id, function, attr, size) \
    { id, function, sys_tsk_##id##_sMemory, size },

// Define the prototype for the function type
typedef void (*task_function)(void);

//!< Define the structure to represent each task in the table
typedef struct TASK_INFO_s{
    uint8_t id;             // Task ID
    task_function func;     // Pointer to the task function
    uint16_t *taskStack;    // Pointer to the task stack memory
    uint8_t size;           // Size of the task
} TASK_INFO_t;

//!< Task control block, implemented as a linked list to point to the TCB of the next task.
typedef struct TCB_s
{
    uint16_t      *stackPt;
    struct TCB_s  *nextPt;
}TCB_t;

//!< Start the scheduler.
void scheduler_start(void);

//!< Give control back to the scheduler
void scheduler_yield(void);

#endif // __SCHEDULER_H__
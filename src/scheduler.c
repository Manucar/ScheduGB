#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "scheduler.h"

POOL_BLOCK_t pool[POOL_SIZE];
POOL_BLOCK_t *freeBlock = NULL;

/// Points to the TCB of the currently active task
TCB_t *pCurntTcb = NULL;

/// Still need to have a struct to hold task info
TCB_t tcbs[NUM_OF_TASKS];


void init_pool(void)
{
    for (int i = 0; i < POOL_SIZE - 1; i++)
    {
        pool[i].nextBlock = &pool[i+1];
    }
    pool[POOL_SIZE - 1].nextBlock = NULL;
    freeBlock = &pool[0];  // Initialize free list head
}

void init_first_task(void *func)
{
    // Request memory
    uint16_t *taskStack = request_block();

    // Init task stack
    taskStack[BLOCK_SIZE - CPU_REGS] = (uint16_t)(taskStack[BLOCK_SIZE - CPU_REGS]);
    taskStack[BLOCK_SIZE - PC_POS] = (uint16_t)(func);

    pCurntTcb->stackPt = taskStack;
    pCurntTcb->nextPt = NULL;
}

uint16_t *request_block(void)
{
    if (freeBlock == NULL)
    {
        printf("No free blocks available!\n");
        return NULL;
    }

    uint16_t *blockToReturn = freeBlock->block;
    freeBlock = freeBlock->nextBlock;
    return blockToReturn;
}

void release_block(uint16_t *block)
{
    if (block == NULL) return;

    POOL_BLOCK_t *releasedBlock = (POOL_BLOCK_t *)block;
    releasedBlock->nextBlock = freeBlock;
    freeBlock = releasedBlock;
}

void create_task(void *func)
{
    uint16_t *taskStack = request_block();

    taskStack[BLOCK_SIZE - CPU_REGS] = (uint16_t)(taskStack[BLOCK_SIZE - CPU_REGS]);
    taskStack[BLOCK_SIZE - PC_POS] = (uint16_t)(func);


    // TCB_t *newTask;
    // newTask->nextPt = pCurntTcb;
    // newTask->stackPt = taskStack;
    pCurntTcb->nextPt = newTask;
}

#if !defined(USE_DYNAMIC_TASKS)
/// Points to the TCB of the currently active task
TCB_t *pCurntTcb;

/// Define an array to store TCB's for the tasks.
TCB_t tcbs[NUM_OF_TASKS];

// Generate static memory arrays for each task
SYS_CFG_TSK_TABLE(STATIC_MEMORY)

// Define the task table using the macro
TASK_INFO_t taskTable[NUM_OF_TASKS] = {
    SYS_CFG_TSK_TABLE(STATIC_TASK_TABLE)
};
#endif //USE_DYNAMIC_TASKS

static void scheduler_sys_tick_handler(void) NONBANKED NAKED
{
    __asm
        // On ISR entry, save registers onto the stack
        push af
        push bc
        push de
        push hl

        // Save actual SP in DE
        ldhl sp, #0x0
        ld d, h
        ld e, l

        // All regs already pushed by entering the ISR, we need only to save SP 
        // into the currentTcb struct
        ld hl, #_pCurntTcb
        ld a, (hl+)
        ld h, (hl)
        ld l, a

        // &stackPt-> *stackPt = SP;
        ld a, (hl+)
        ld h, (hl)
        ld l, a 
      
        // Save SP saved in DE into *HL
        ld a, e
        ld (hl+), a
        ld a, d
        ld (hl), a
        
////////////////////////////////////////////////////////////////////////////////
        // Restore the context from the next scheduled task
        ld hl, #_pCurntTcb // Reload and re-dereference

        // Dereference pCurntTcb to pointed tcb
        ld a, (hl+)
        ld h, (hl)
        ld l, a

        // Increase pointer to next task
        ld bc, #0x0002
        add hl, bc

        // Dereference pointer to next task
        ld a, (hl+)
        ld h, (hl)
        ld l, a

        // Save pointer to new task to current stack pointer
        ld a, l
        ld (#_pCurntTcb), a
        ld a, h
        ld (#_pCurntTcb+1), a

////////////////////////////////////////////////////////////////////////////////
        // Dereferce it to obtain &SP
        ld a, (hl+)
        ld h, (hl)
        ld l, a

        // Dereferce it to obtain SP
        ld a, (hl+)
        ld h, (hl)
        ld l, a

        // Load SP value into SP register
        ld sp, hl
        
        // Pop all regs from new stack frame
        pop hl
        pop de
        pop bc
        pop af

        // Finally pop PC from the stack into actual PC reg to jump there (indirectly)
        reti

    __endasm;
}

static void scheduler_set_sys_tick(void)
{
    CRITICAL
    {
        add_TIM(scheduler_sys_tick_handler);
    }
    // Set TMA to divide clock by 0x100
    TMA_REG = 0x00U;
    // Set clock to 4096 Hertz
    TAC_REG = 0x04U;

    // Handle VBL and TIM interrupts
    set_interrupts(TIM_IFLAG);
}

#if !defined(USE_DYNAMIC_TASKS)
static void scheduler_init_thread_stack(void) CRITICAL
{
    // Init each stack with known pattern 0xCD
    for (uint8_t i = 0; i < NUM_OF_TASKS; i++)
    {
        memset(taskTable[i].taskStack, 0xCD, taskTable[i].size);
    }
    
    for (uint8_t i = 0; i < NUM_OF_TASKS; i++)
    {

        tcbs[i].nextPt = &tcbs[(i+1) % NUM_OF_TASKS];
        tcbs[i].stackPt = &taskTable[i].taskStack[taskTable[i].size - CPU_REGS];

        taskTable[i].taskStack[taskTable[i].size - CPU_REGS] = (uint16_t)(&taskTable[i].taskStack[taskTable[i].size - CPU_REGS]);
        taskTable[i].taskStack[taskTable[i].size - PC_POS] = (uint16_t)(taskTable[i].func);
    }

    /// Make current tcb pointer point to the first task
    pCurntTcb = &tcbs[0];
}
#endif // USE_DYNAMIC_TASKS

static void scheduler_launch_first_task(void) NONBANKED NAKED CRITICAL
{
    __asm
        // Load pCurntTcb into SP. has to be dereferenced 2 times 
        // &pCurntTcb-> *pCurntTcb = &stackPt;
        ld hl, #_pCurntTcb
        ld a, (hl+)
        ld h, (hl)
        ld l, a

        // &stackPt-> *stackPt = SP;
        ld a, (hl+)
        ld h, (hl)
        ld l, a                 

        ld sp, hl
        
        // Pop all regs from there
        pop hl
        pop de
        pop bc
        pop af

        // Finally pop PC from the stack into actual PC reg to jump there (indirectly)
        ret

    __endasm;
}

// Yield control back to scheduler by overflowing timer
void scheduler_yield(void)
{
    TIMA_REG = 0xFFU;
}

void scheduler_start(void)
{
#if !defined(USE_DYNAMIC_TASKS)
    scheduler_init_thread_stack();
#endif
    scheduler_set_sys_tick();

    scheduler_launch_first_task();
}

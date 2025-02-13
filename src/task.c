#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define POOL_SIZE 5
#define BLOCK_SIZE 16

typedef struct POOL_BLOCK_s
{
    uint8_t block[BLOCK_SIZE];
    struct POOL_BLOCK_s * nextBlock;
} POOL_BLOCK_t;

POOL_BLOCK_t pool[POOL_SIZE];
POOL_BLOCK_t *freeBlock = NULL;

void init_pool(void)
{
    for (int i = 0; i < POOL_SIZE - 1; i++)
    {
        pool[i].nextBlock = &pool[i+1];
    }
    pool[POOL_SIZE - 1].nextBlock = NULL;
    freeBlock = &pool[0];  // Initialize free list head
}

uint8_t *request_block(void)
{
    if (freeBlock == NULL)
    {
        printf("No free blocks available!\n");
        return NULL;
    }

    uint8_t *blockToReturn = freeBlock->block;
    freeBlock = freeBlock->nextBlock;
    return blockToReturn;
}

void release_block(uint8_t *block)
{
    if (block == NULL) return;

    POOL_BLOCK_t *releasedBlock = (POOL_BLOCK_t *)block;
    releasedBlock->nextBlock = freeBlock;
    freeBlock = releasedBlock;
}

int main(void)
{
    init_pool();
    uint8_t *tmpBlock;
    
    for (int i = 0; i < 3; i++)
    {
        tmpBlock = request_block();
        printf("Requested block address = %p\n", (void *)tmpBlock);
        
        if (tmpBlock)
        {
            for (int j = 0; j < BLOCK_SIZE; j++)
            {
                tmpBlock[j] = (i * j) + j; 
                printf("%d ", tmpBlock[j]);
            }
            printf("\n");
        }
    }

    release_block(tmpBlock);
    
    uint8_t *tmpBlock1 = request_block();
    printf("Requested block address tmpBlock1 = %p\n", (void *)tmpBlock1);
    
    if (tmpBlock1)
    {
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            printf("%d ", tmpBlock1[j]);
        }
        printf("\n");
    }

    return 0;
}

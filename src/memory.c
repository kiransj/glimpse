#include <display.h>
#include <paging.h>


#define MEMORY_ALIGN    32

#define NUM_32_BLOCKS   127 


struct list32
{    
    uint8_t address[0x1000 - MEMORY_ALIGN];
    uint8_t bitmap[16];    
    struct list *next;
    uint8_t reserved[12];
};


uint32_t allocate_block_32(struct list32 *root)
{
    uint32_t i;
    uint8_t flag = 0xff;
    for(i = 0; i < 16; i++)
    {
        if((root->bitmap[i] & flag) != flag)
        {
            uint8_t tmp = 0x1, j;
            for(j = 0; j < 8; j++)
            {
                if((root->bitmap[i] & (tmp << j)) == 0)
                {
                    uint8_t block_number =  (i << 3) + j;
                    root->bitmap[i] |= (tmp << j);
                    LOG_INFO("block number : %d", block_number);
                    return (uint32_t)((uint32_t)&(root->address) + (block_number<<5));
                }
            }
        }
    }

    LOG_ERROR("NO Free Blocks of size 32, what to do!!!");
    asm volatile ("cli");
    while(1);
}

void free_block_32(struct list32 *root, uint32_t address)
{
    uint32_t block = (address - (uint32_t)(&root->address))>>5;
    uint8_t idx, offset, flag = 1;

    LOG_INFO("free block: %x, %x", block, address);
    idx = (block >> 3) & 0x1f;
    offset = block & (0x7);

    if((root->bitmap[idx] & (flag << offset)) != 0)
    {
        root->bitmap[idx] &= ~(flag << offset);
    }
    else
    {
        LOG_ERROR("trying to free unallocated block");
    }    
}

struct list32 *root32;
void malloc_initialize(void)
{
    uint32_t address = get_mapped_page();
    LOG_INFO("sizeof(struct list) = %d\n", sizeof(struct list32));

    root32 = (struct list32 *)(address);
    memset(root32, 0, 0x1000);

    LOG_INFO("Address : %x", root32->address);
    uint32_t add = allocate_block_32(root32);
    uint32_t add1 = allocate_block_32(root32);
    LOG_INFO("allocated address : %x\n", add);
    free_block_32(root32, add1);
    free_block_32(root32, add);

    return ;
}

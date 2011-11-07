#include <display.h>
#include <paging.h>

#define MEMORY_ALIGN    32
struct list32
{    
    uint8_t bitmap[16];
    uint8_t address[4096 - MEMORY_ALIGN];
    struct list *next;
    uint8_t reserved[12];
};

struct list *root;
void malloc_initialize(void)
{
    uint32_t address = get_mapped_page();
    LOG_INFO("sizeof(struct list) = %d\n", sizeof(struct list));

    root = (struct list *)(address);
    LOG_INFO("Address : %x", address);
    LOG_INFO("root->bitmap = %x",  root->bitmap);
    LOG_INFO("root->address = %x", root->address);
    LOG_INFO("root->next = %x",  &(root->next));
}

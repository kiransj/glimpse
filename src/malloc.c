#include <malloc.h>
#include <display.h>

#include <asm.h>
#define PAGE_SIZE	4096

#define FLAG_BLOCK_ACCUPIED     0x1

void print_mem_list(void);
typedef struct block_header
{
	uint32_t address;
	uint32_t size:24;
	uint8_t flags;
	struct block_header *next;
    struct block_header *prev;
}*BlockHeader;

typedef struct mem_list
{
	BlockHeader block;
	uint32_t biggest_block_size;
	uint32_t block_starting_address;
    uint32_t size;
	bool mainBlock;
	struct mem_list *next;
}*MemList;

bool is_block_free(BlockHeader block)
{
    return ((block->flags & FLAG_BLOCK_ACCUPIED) == FLAG_BLOCK_ACCUPIED) ? false : true;
}

void set_block_accupied(BlockHeader block, bool flag)
{
	if(true == flag)
	{
		if(is_block_free(block))
		{
			block->flags |= (FLAG_BLOCK_ACCUPIED);
		}
		else
		{
			LOG_WARN("Warning : block is already accupied\n");
		}
	}
	else
	{
		block->flags &= ~(FLAG_BLOCK_ACCUPIED);
	}
}

bool address_within_this_list(MemList list, uint32_t address)
{
	return (address >= list->block_starting_address) && (address < (list->block_starting_address+list->size));
}
bool address_within_this_block(BlockHeader block, uint32_t address)
{
	return (address >= block->address) && (address < (block->address+block->size));
}

MemList root;

void initialize_block_headers(BlockHeader block, uint32_t address, uint32_t size)
{
	block->address = address;
	block->size = size;
	block->flags = 0;
	block->next = block->prev = NULL;
}

void kmalloc_initialize(const uint32_t addre, const uint32_t size)
{
    uint32_t address = addre;
    root = (MemList)(address);
    root->size =  size;
    root->block_starting_address = address;
	root->mainBlock = true;
	root->next = NULL;
    address = (uint32_t)root+sizeof(struct mem_list);
	root->block = (BlockHeader)address;
	root->biggest_block_size =size  - sizeof(struct mem_list);
	address += sizeof(struct block_header);
	initialize_block_headers(root->block, address, root->biggest_block_size);

    print_mem_list();
    return;
}

/*Size should be inclusive of the block_header*/
void split_block(BlockHeader block, uint32_t size)
{
	/*Cbeck if we really need to split this block*/
	if(block->size > (size + sizeof(struct block_header) + 16))
	{
		BlockHeader bh = (BlockHeader)(block->address + size);
		initialize_block_headers(bh, block->address+size, block->size - size);
		bh->next = block->next;
		bh->prev = block;
		if(NULL != block->next)
		block->next->prev = bh;
		block->next = bh;
		block->size = size;
	}
	return;
}


void* kmalloc(const uint32_t size)
{
	uint32_t tmp_size;
	MemList list = root;
	BlockHeader tmp = list->block, tmp1 = NULL;

	tmp_size = ((size < 16)? 16 : size) + sizeof(struct block_header);
    LOG_INFO("Allocate : %d, %d\n", size, tmp_size);
	while(NULL != tmp)
	{
		if((true == is_block_free(tmp)) && (tmp_size <= tmp->size))
		{
			if(NULL == tmp1)
			{
				tmp1 = tmp;
			}
			else
			{
				if(tmp->size < tmp1->size)
					tmp1 = tmp;
			}
		}
		tmp = tmp->next;
	}

	tmp = tmp1;
	if(NULL != tmp)
	{
		split_block(tmp, tmp_size);
		set_block_accupied(tmp, true);
		return (void*)(tmp->address + sizeof(struct block_header));
	}

    print_mem_list();
    LOG_ERROR("No More Memory to allocate!!!!, stoping everything");
    CLEAR_INTERRUPT();
    while(1);
	return NULL;
}

void join_block(BlockHeader block)
{
	if((NULL != block->next) && is_block_free(block->next))
	{
		BlockHeader tmp = block->next;
		block->size += tmp->size;
		block->next = tmp->next;
		if(NULL != tmp->next)
		{
			tmp->next->prev = block;
		}
	}
	if((NULL != block->prev) && is_block_free(block->prev))
	{
		BlockHeader tmp = block->prev;
		tmp->size += block->size;
		tmp->next = block->next;
		if(NULL != block->next)
		{
			block->next->prev = tmp;
		}
		block = tmp;
	}
}

void kfree(void *addr)
{
	uint32_t address = (uint32_t)addr;
 	MemList list = root;
	BlockHeader tmp;
	while(NULL != list)
	{
		if(address_within_this_list(list, address))
		{
			tmp = list->block;
			while(NULL != tmp)
			{
				if(address_within_this_block(tmp, address))
				{
					if(!is_block_free(tmp))
					{
						set_block_accupied(tmp, false);
						join_block(tmp);
					}
					else
					{
						LOG_ERROR("address not found\n");
					}
					return;
				}
				tmp = tmp->next;
			}
			return;
		}
		list = list->next;
	}
	LOG_ERROR("address not found\n");
	return;
}

void print_mem_list(void)
{
  	MemList list = root;
	while(NULL != list)
	{
		BlockHeader tmp = list->block;
		LOG_INFO("<-MemList : MainBlock(%d), address(%d), size(%d) ->\n", list->mainBlock, list->block_starting_address, list->size);
		while(NULL != tmp)
		{
			LOG_INFO("\t<-BlockList: Address(%d), size(%d), flag(%d) ->\n", tmp->address, tmp->size, tmp->flags);
			tmp = tmp->next;
		}
		list = list->next;
	}
}
#if 0
int main(int argc, char *argv[])
{
	uint32_t i, j;
	void **address;
	malloc_initialize();
	address = (void**)kmalloc(sizeof(void*)*10);
	memset(address, 0, sizeof(void*)*4);
	print_mem_list();

	for(i = 0, j = 0; i < 10; i++)
	{
		address[i] = kmalloc(100*(10 - i));
		memset(address[i], 0, 100*(10 - i));
	}
	print_mem_list();
	kfree(address[1]);
	kfree(address[3]);
	kfree(address[5]);
	kfree(address[7]);
	kfree(address[9]);
	print_mem_list();
	kfree(address[2]);
	print_mem_list();
}
#endif

#include "main.h"
#include "screen.h"

#define		PAGE_BIT		21
#define		PAGE_SIZE		0x200000
#define		KERNEL_PAGES	10
#define		HEAP_BASE		(PAGE_SIZE * KERNEL_PAGES) //0X140 0000
#define		MAX_MEM			(PAGE_SIZE * 256)

QWORD pyshical_page_bitmap[4] = { 0 };// 0b1111111111; //first 3 used by kernel

#define QW_OFFEST(b) ((b) / 64)
#define BIT_OFFSET(b)  ((b) % 64)

void setBits(BYTE f, BYTE l) {
	for (BYTE i = f; i <= l; i++)
		pyshical_page_bitmap[QW_OFFEST(i)] |= ((QWORD)1 << BIT_OFFSET(i));
}

void resetBits(BYTE f, BYTE l) {
	for (BYTE i = f; i <= l; i++)
		pyshical_page_bitmap[QW_OFFEST(i)] &= ~((QWORD)1 << BIT_OFFSET(i));
}

int checkBits(BYTE f, BYTE l) {
	for (BYTE i = f; i <= l; i++)
		if ((pyshical_page_bitmap[QW_OFFEST(i)] & ((QWORD)1 << BIT_OFFSET(i))) == 0)
			return 0;
	return 1;
}

QWORD alloc_pyshical_pages(BYTE n, QWORD address) {	
	BYTE i;
	i = address / PAGE_SIZE;
	QWORD pageMask = (((QWORD)1 << n) - 1) << i;
	if (checkBits(i, i + n - 1))
		return 0;
	setBits(i, i + n - 1);
	return 1;
	/*BYTE i;
	QWORD pageMask = ((QWORD)1 << n) - 1;

	for (i = KERNEL_PAGES; i < 256 - n; i++) {
		QWORD mask = pageMask << i;q
		if ((mask & pyshical_page_bitmap) == 0) //page is empty
		{
			pyshical_page_bitmap |= mask;
			QWORD address = (QWORD)PAGE_SIZE * i;
			return address;
		}
	}*/
}

void free_pyshical_page(QWORD address) {
	BYTE i;

	i = address / PAGE_SIZE;
	resetBits(i, i);
	//pyshical_page_bitmap &= ~((QWORD)1 << i);
}

QWORD is_pyshical_page(QWORD address) {
	BYTE i;

	i = address / PAGE_SIZE;
	return (checkBits(i, i));
	//return (pyshical_page_bitmap & ((QWORD)1 << i));
}

enum {
	FREE,
	USED
};

typedef struct s_mem_desc{ //Describe a free space
	struct s_mem_desc* next, * prev;
	QWORD	base, size;
	BYTE	used_desc;
}MEM_DESC;

MEM_DESC mem_desc_table[1000];//allocate 1000 descriptors

MEM_DESC* first_free;
MEM_DESC* first_used;
QWORD	 higher_pysical_address;

MEM_DESC* get_new_desc() {
	for (int i = 0; i < 1000; i++)
		if (mem_desc_table[i].used_desc == 0) {
			mem_desc_table[i].used_desc = 1;
			return &mem_desc_table[i];
		}
	return 0;
}

void free_if_needed(QWORD size, QWORD base) {
	//NB(base); STR(" -- "); NB(size); STR("\n");
	QWORD i = ((base - 1) & ~(PAGE_SIZE - 1)) + PAGE_SIZE;
	QWORD n = (base + size - 1) & ~(PAGE_SIZE - 1);

	//NB(i / PAGE_SIZE); STR(" -- "); NB(n / PAGE_SIZE); STR("\n");
	while (i <= n) {
		if (is_pyshical_page(i))
			free_pyshical_page(i);
		i += PAGE_SIZE;
	}
	
}

void add_to_free_list(MEM_DESC* f_free) {
	MEM_DESC* f = first_free;
	if (f->base > f_free->base) {//insert in first pos
		f_free->next = f;
		f_free->prev = 0;
		f->prev = f_free;
		first_free = f_free;
		f = f_free;
	}
	else {
		while (f->next) {
			if (f->next->base > f_free->base)
				break;
		}
		//at the end or befor one
		if (f->next) { //putting befor next
			f_free->next = f->next;
			f->next->prev = f_free;
		}
		f->next = f_free; //putting affter current
		f_free->prev = f;
	}

	MEM_DESC* f2, *f3;
	f2 = f->next;
	f3 = f2->next;
	if (f3) {
		if (f2->base + f2->size == f3->base) //join 2 and 3
		{
			f2->size += f3->size;
			f2->next = f3->next;
			if (f3->next)
				f3->next->prev = f2;
			f3->used_desc = 0;
		}
	}
	//join 1 and 2
	if (f->base + f->size == f2->base) {
		f->size += f2->size;
		f->next = f2->next;
		if (f2->next)
			f2->next->prev = f;
		f2->used_desc = 0;
	}

	free_if_needed(f->size, f->base);
}

void init_memory() {
	alloc_pyshical_pages(1, HEAP_BASE);
	first_free = &mem_desc_table[0];
	first_used = &mem_desc_table[1];
	first_used->base = HEAP_BASE;
	first_used->size = 1;
	first_used->used_desc = 1;
	first_used->prev = first_used->next = 0;

	first_free->base = HEAP_BASE + 1;
	first_free->size = MAX_MEM - 1;
	first_free->used_desc = 1;
	first_free->prev = first_used->next = 0;
}

QWORD physical_alloc_if_needed(QWORD size, QWORD base) {
	QWORD i = base & ~(PAGE_SIZE - 1);
	QWORD n = (base + size - 1) & ~(PAGE_SIZE - 1);
	while (i <= n) {
		if (!is_pyshical_page(i))
			if (0 == alloc_pyshical_pages(1, i))
				return 0;
		i += PAGE_SIZE;
	}
	return 1;
}

void delete_from_list(MEM_DESC** head, MEM_DESC* f) {
	if (f->prev) {
		f->prev->next = f->next;
		if (f->next)
			f->next->prev = f->prev;
	}
	else {
		*head = f->next;
		if (f->next)
			f->next->prev = 0;
	}
}

QWORD my_malloc(QWORD n) {
	MEM_DESC* f = first_free;
	while (f) {
		if (f->size >= n) { //Found free memory
			if (0 == physical_alloc_if_needed(n, f->base))
				return 0;
			MEM_DESC* f_used = get_new_desc();
			//Insert desc in allocated list desc
			f_used->base = f->base;
			f_used->size = n;
			f_used->prev = NULL;
			f_used->next = first_used;
			first_used->prev = f_used;
			first_used = f_used;
			//Refactor free list;
			if (f->size > n) {
				f->size -= n;
				f->base += n;
			}
			else {
				delete_from_list(&first_free, f);
				f->used_desc = 0;
			}
			return f_used->base;
		}
		f = f->next;
	}
	return 0;
}
void  my_free(QWORD addr) {
	MEM_DESC* f = first_used;
	while (f) {
		if (f->base == addr) { //Found free memory
			MEM_DESC* f_free = get_new_desc();
			f_free->base = addr;
			f_free->size = f->size;
			add_to_free_list(f_free);

			delete_from_list(&first_used, f);
			f->used_desc = 0;
		}
		f = f->next;
	}
}

int isAllocatedAddres(QWORD addr) {
	MEM_DESC* f = first_used;
	while (f) {
		if (addr >= f->base && (addr < (f->base + f->size)))
			return 1;
		f = f->next;
	}
	return 0;
}

int  writeQ(QWORD addr, QWORD n) {
	if (isAllocatedAddres(addr)) {
		*(QWORD*)addr = n;
		return 1;
	}
	return 0;
}
int  readQ(QWORD addr, QWORD* n) {
	if (isAllocatedAddres(addr)) {
		*n = *(QWORD*)addr;
		return 1;
	}
	return 0;
}


void printDesc() {
	MEM_DESC* f = first_free;
	STR("Free descriptors: \n");
	while (f) {
		STR("Base: "); NB(f->base); STR(" Size:");  NB(f->size); STR("\n");
		f = f->next;
	}
	f = first_used;
	STR("Allocated descriptors: \n");
	while (f) {
		STR("Base: "); NB(f->base); STR(" Size:");  NB(f->size); STR("\n");
		f = f->next;
	}
	STR("Bitmap: "); NB(pyshical_page_bitmap[0]); STR("\n");
}
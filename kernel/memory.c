#include "main.h"
#include "screen.h"

#define MAX_ADRESABLE_SIZE		0x3FFFFFFF
#define	BASE_PAGE_TABLE			0xA000
#define	TABLE_ENTRIES			256 // One entry 2Mb -> 512 MB == Physical memory of the system
#define BASE_UNALLOCATE_MEMORY	0x600000
#define PAGE_SIZE				0x200000
#define PAGE_SIZE_BIT			21 //1 << 21
#define RAM_SIZE				512
#define KERNEL_ENTRIES			3
#define HEAP_BASE				0x20000000

#define PRESENT_AND_WRITEABLE   0x3
#define HUGE_PAGE               (1 << 7)

QWORD last_ram_mem = BASE_UNALLOCATE_MEMORY;

QWORD* last_table_entry = (QWORD*)(BASE_PAGE_TABLE + KERNEL_ENTRIES * sizeof(QWORD));
QWORD  last_allocated_address = BASE_UNALLOCATE_MEMORY;
/* 
	Allocates next avalible physical memory.
	Map 1:1 with virtual memory
	Size must be multiple of 2MB, else the allocated size
	will be the next aligned size
*/
QWORD allocate_physical_memory(QWORD pages) {
	if (pages == 0)
		return 0;
	if (last_allocated_address + (PAGE_SIZE * pages) > HEAP_BASE) //No mor space for physical memory
		return 0;

	QWORD start_address = last_allocated_address;
	last_allocated_address += PAGE_SIZE * pages;

	QWORD entry;
	while (pages > 0) {
		entry = last_ram_mem;
		last_ram_mem += PAGE_SIZE;
		entry |= PRESENT_AND_WRITEABLE | HUGE_PAGE;
		*last_table_entry = entry;
		last_table_entry++;
		pages--;
	}
	return start_address;
}

QWORD* last_heap_entry = (QWORD*)(BASE_PAGE_TABLE + 256 * sizeof(QWORD)); //Entry for HEAP_BASE address
QWORD  last_heap_address = HEAP_BASE;
/*
	Allocates next avalible pages in heap.
*/
QWORD allocate_virtual_memory(QWORD pages) {
	QWORD size = pages * PAGE_SIZE;
	if (last_heap_address + size > MAX_ADRESABLE_SIZE) //No mor space for virtual memory
		return 0;

	QWORD start_address = last_heap_address;
	last_heap_address += size;

	QWORD entry;
	while (pages > 0) {
		entry = last_ram_mem;
		last_ram_mem += PAGE_SIZE;
		entry |= PRESENT_AND_WRITEABLE | HUGE_PAGE;
		*last_heap_entry = entry;
		last_heap_entry++;
		pages--;
	}

	return start_address;
}

QWORD  last_allocated = HEAP_BASE;
/*
	Allocate the specified size on the allocated heap
*/
QWORD allocate_in_heap(QWORD size) {
	QWORD start_address = last_allocated;
	if (last_allocated + size > last_heap_address) {
		QWORD size_to_allocate = last_allocated + size - last_heap_address;
		if (size_to_allocate & (PAGE_SIZE - 1))
		{
			size_to_allocate &= ~(PAGE_SIZE - 1);
			size_to_allocate += PAGE_SIZE;
		}
		if (allocate_virtual_memory(size_to_allocate >> PAGE_SIZE_BIT) == 0)
			return 0;
	}

	last_allocated += size;
	return start_address;
}

/* We allocate memory and if there is a triple fault something was wrong */
void test_memory_allocation() {
	QWORD addr;
	QWORD size;
	QWORD i;


	//BREAK("Befor test");
	size = PAGE_SIZE * 1;
	addr = allocate_physical_memory(1);
	for (i = 0; i < size; i++)
		((BYTE*)addr)[i] = 'a';
	//BREAK("TEst1 OK");

	size = PAGE_SIZE * 2;
	addr = allocate_physical_memory(2);
	for (i = 0; i < size; i++)
		((BYTE*)addr)[i] = 'a';
	//BREAK("TEst2 OK");

	size = 10;
	addr = allocate_in_heap(size);
	for (i = 0; i < size; i++)
		((BYTE *)addr)[i] = 'a';
	//BREAK("TEst3 OK");
	
	size = PAGE_SIZE * 2;
	addr = allocate_in_heap(size);
	for (i = 0; i < size; i++)
		((BYTE*)addr)[i] = 'a';
	//BREAK("TEst4 OK");


	//BREAK("Befor test");
	size = PAGE_SIZE * 1;
	addr = allocate_physical_memory(1);
	for (i = 0; i < size; i++)
		((BYTE*)addr)[i] = 'a';
	//BREAK("TEst5 OK");

	size = PAGE_SIZE;
	addr = allocate_in_heap(size);
	for (i = 0; i < size; i++)
		((BYTE*)addr)[i] = 'a';
	//BREAK("TEst3 OK");
}
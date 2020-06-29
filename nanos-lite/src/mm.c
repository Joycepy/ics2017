#include "proc.h"
#include "memory.h"
extern void _map(_Protect *as, void *vs, void *pa);
static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
	if(current->cur_brk==0){
	current->cur_brk = current->max_brk = new_brk;
  }
  else{
	if(new_brk > current->max_brk){
		// map memory region [current->max_brk, new_brk)
		// into address space current->as
		int size = new_brk - current->max_brk;
		void *page;
		void* va = (void*)PGROUNDUP(current->max_brk); 
		for(int i=0;i<size;i+=PGSIZE){
			page = (void*)new_page();
			_map(&current->as, va + i, page);
		}

		current->max_brk = new_brk;	
	}
	current->cur_brk=new_brk;
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}

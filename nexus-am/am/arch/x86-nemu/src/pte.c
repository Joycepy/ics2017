#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  //将虚拟地址空间 p 中的虚拟地址 va 映射到物理地址 pa
  void* addrPDE = p->ptr;
	PDE* basePDE = (PDE*) addrPDE;
	
	uint32_t dir = PDX(va);
	uint32_t page = PTX(va);
	if(!(basePDE[dir] & 0x1)){
		PTE* basePTE = (PTE*)palloc_f();
		basePDE[dir] = (uint32_t)basePTE | PTE_P;
		basePTE = (PTE*)(basePDE[dir] & 0xfffff000);
		basePTE[page] = (uint32_t)pa | PTE_P;
	}
	else{
		PTE* basePTE = (PTE*)(basePDE[dir] & 0xfffff000);
		basePTE[page] = (uint32_t)pa | PTE_P;	
	}
  // PDE *pgdir = (PDE *)(p->ptr);
  // PDE *pde = &pgdir[PDX(va)];
  // PTE *pgtable;
	// if (!(*pde & 0x1)){
  //   pgtable=(PTE*)palloc_f();
  //   int i;
  //   for(i=0;i<NR_PTE;i++)
  //     pgtable[i]=0;
	// 	*pde  = PTE_ADDR(pgtable) | PTE_P;
  // }
  // else{
  //   pgtable=(PTE*)PTE_ADDR(*pde);
  // }
	// pgtable[PTX(va)] = PTE_ADDR((uint32_t)pa) | PTE_P;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
	uint32_t* ptr = (uint32_t*)(ustack.end-4);
	*(ptr--)=0;
	*(ptr--)=0;
	*(ptr--)=0;
	*(ptr--)=0;

	_RegSet* tf = (void*)(ptr-sizeof(_RegSet));
	tf->eflags = 0x202;
	tf->cs=8;
	tf->eip=(uintptr_t)entry;
	
  return tf;
}

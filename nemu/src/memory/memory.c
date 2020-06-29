#include "nemu.h"
#include "device/mmio.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id=is_mmio(addr);
  if(mmio_id!=-1)
  {
	return mmio_read(addr,len,mmio_id);
  }
  else
	return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_id=is_mmio(addr);
  if(mmio_id!=-1)
  {
	mmio_write(addr,len,data,mmio_id);
  }
  else
	memcpy(guest_to_host(addr), &data, len);
}

uint32_t page_translate(vaddr_t addr,bool iswrite)
{
  paddr_t pde;
  paddr_t pte;
  paddr_t base1=cpu.cr3.val;
  //Log("pagefram:0x%x",pde_base);
  paddr_t pde_address=base1+((addr>>22)<<2);
  pde=paddr_read(pde_address,4);
  if(!(pde & 0x1))
  {
    Log("vaddr: 0x%x, pde_address: 0x%x",addr,pde_address);
    Log("pde.val:0x%x",pde);
    assert(0);
  } 
  paddr_t base2=pde &0xfffff000;
  //paddr_t pte_address = base2 + ((addr>>12) & 0x3ff)<<2;
  paddr_t pte_address=base2+((addr&0x003ff000)>>10);
  pte =paddr_read(pte_address,4);
  if(!(pte & 0x1))
  {
    Log("vaddr: 0x%x, pte_address: 0x%x",addr,pte_address);
    Log("pte.val:0x%x",pte);
    assert(0); 
  }
      
  paddr_t paddress=(pte&0xfffff000)+(addr&0xfff);

  // set the access and dirty
  pde = pde | 0x20;
  pte = pte | 0x20;
  if (iswrite) {
    pde = pde | 0x40;
    pte = pte | 0x40;
  }
  paddr_write(pde_address,4,pde);
  paddr_write(pte_address,4,pte); 
  return paddress;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging){
    if(((addr&0xfff)+len)>4096){
      //Log("Read cross the page boundry.");
      //assert(0);
      paddr_t paddr,low,high;
      int x;
      x=(int)((addr&0xfff)+len-4096);
      paddr=page_translate(addr,false);
      low=paddr_read(paddr,len-x);
      paddr=page_translate(addr+len-x,false);
      high=paddr_read(paddr,x);
      paddr=(high<<((len-x)<<3))+low;
      //if(addr>0x8048000)
        //printf("vaddr: %x, content: %x\n",addr,paddr);
      return paddr;
    } 
    else  {
      paddr_t paddr=page_translate(addr,false);
      paddr= paddr_read(paddr,len);
      //if(addr>0x8048000)
        //printf("vaddr: %x, paddr: %x\n",addr,paddr);
      return paddr;
    }  
  }  
  else
    return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging){ 
    if(((addr&0xfff)+len)>4096){ 
      //Log("Write cross the page boundry.");
      //assert(0);
      paddr_t paddr,low,high;
      int x;
      x=(int)((addr&0xfff)+len-0x1000);
      low=(data<<(x<<3))>>(x>>3);
      high=data>>((len-x)<<3);
      paddr=page_translate(addr,true);
      paddr_write(paddr,len-x,low);

      paddr=page_translate(addr+len-x,true);
      paddr_write(paddr,x,high);
    }   
    else{
    paddr_t paddr=page_translate(addr,true);
    paddr_write(paddr,len,data);
    }    
  }  
  else
    paddr_write(addr, len, data);
}



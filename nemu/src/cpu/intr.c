#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  //保存现场
  rtl_push(&cpu.eflags.val);
  rtl_push((rtlreg_t *)&cpu.cs);
  rtl_push(&ret_addr);

  //printf("sizeof(GateDesc) = %d",sizeof(GateDesc));//=8
  //下一步执行
  uint32_t idtr_base = cpu.idtr.base;
  uint32_t eip_low, eip_high, offset;
  eip_low=vaddr_read(idtr_base+NO*8,4)&0x0000ffff;
  eip_high = vaddr_read(idtr_base + NO * 8 + 4, 4) & 0xffff0000;
  offset = eip_low | eip_high;
	decoding.jmp_eip = offset;
	decoding.is_jmp = true;
  //pa4.3.处理器进入关中断状态.
  cpu.eflags.IF = 0;
}

void dev_raise_intr() {
  cpu.INTR = true;
}

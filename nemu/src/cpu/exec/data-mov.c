#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(movsb) {
  rtl_lr(&t0,R_ESI,4);
  rtl_lm(&t1,&t0,1);
  t0+=1;
  rtl_sr(R_ESI,4,&t0);
  rtl_lr(&t0,R_EDI,4);
  rtl_sm(&t0,1,&t1);
  t0+=1;
  rtl_sr(R_EDI,4,&t0);
  print_asm_template1(movsb);
}


make_EHelper(push){
  //TODO();
  //push imm8指令需要对立即数进行符号扩展，译码阶段完成
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  //TODO();
  rtl_pop(&t2);
  operand_write(id_dest,&t2);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  t2=cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);

  rtl_push(&t2);

  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t2);

  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  cpu.esp=cpu.ebp;
  rtl_pop(&t0);
  cpu.ebp=t0;

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    //TODO();
	//DX:AX ← sign-extend of AX
	rtl_lr_w(&t0,R_AX);//R_AX: reg.h enum
	rtl_sext(&t0,&t0,2);
	rtl_sari(&t0,&t0,31);
	rtl_sr_w(R_DX,&t0);
  }
  else {
    //TODO();
	//EDX:EAX ← sign-extend of EAX
	//rtl_lr_l(&t0,R_EAX);
	//rtl_sari(&t0,&t0,32);
	rtl_sari(&cpu.edx,&cpu.eax,31);
	//rtl_sari(&t0,&t0,1);
	//rtl_sr_l(R_EDX,&t0); 
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    //TODO();
	//AX ← SignExtend(AL);
	rtl_lr_b(&t0,R_AX);
	rtl_sext(&t0,&t0,1);
	rtl_sr_w(R_AX,&t0);
  }
  else {
    //TODO();
	//EAX ← SignExtend(AX);
	rtl_lr_w(&t0,R_AX);
	rtl_sext(&t0,&t0,2);
	rtl_sr_l(R_EAX,&t0);
  }
  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}

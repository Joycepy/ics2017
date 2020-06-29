#include "common.h"
#include "syscall.h"
#include "fs.h"

extern int mm_brk(uint32_t new_brk);
 /* static inline uintptr_t sys_write(uintptr_t fd,void* buf,uintptr_t len)
{
  Log("sys_write:fd %d  len %d",fd,len);  
  return fs_write((int)fd,(void*)buf,(size_t)len);
 uintptr_t i=0;
  if(fd==1||fd==2)
  {
    for(;i<len;i++)
      _putc(((char*)buf)[i]);
  }
  //On success, the number of bytes written is returned.
  return i;
}
*/

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      SYSCALL_ARG1(r)=1;
      break;
    case SYS_exit:
      _halt(a[1]);
      break;
    case SYS_write:
      SYSCALL_ARG1(r)=fs_write((int)a[1],(void *)a[2],(size_t)a[3]);
      break;
    case SYS_brk:
    //3.2还是一个单任务操作系统,空闲的内存都可以让用户程序自由使用,因此返回0,表示堆区大小的调整总是成功. 
      SYSCALL_ARG1(r)=mm_brk(a[1]);
      break;
    case SYS_open:
      SYSCALL_ARG1(r)=fs_open((char*)a[1],(int)a[2],(int)a[3]);
      break;
    case SYS_read:
      SYSCALL_ARG1(r)=fs_read((int)a[1],(void *)a[2],(size_t)a[3]);
      break;
    case SYS_lseek:
      SYSCALL_ARG1(r)=fs_lseek((int)a[1],(off_t)a[2],(int)a[3]);
      break;
    case SYS_close:
      SYSCALL_ARG1(r)=fs_close((int)a[1]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}

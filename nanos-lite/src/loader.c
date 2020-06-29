#include "common.h"
#include "memory.h"
#include "fs.h"
size_t get_ramdisk_size();
void ramdisk_read(void *,off_t,size_t);
extern void _map(_Protect *p, void *va, void *pa);
#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //pa3.1中没有文件系统，程序的加载就是磁盘读写
  //ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());

  //pa3.2 文件系统,测试程序/bin/text
  /* int fd=fs_open("/bin/pal",0,0);
  int size=fs_filesz(fd);
  fs_read(fd,DEFAULT_ENTRY,size);*/

  int fd=fs_open(filename,0,0);
  int size=fs_filesz(fd);
  void *page;

  for(int i=0;i<size;i+=PGSIZE){
	page = (void*)new_page();
	_map(as, DEFAULT_ENTRY + i, page);
	fs_read(fd, page, PGSIZE);
  }
  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}

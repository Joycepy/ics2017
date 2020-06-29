#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size=_screen.width*_screen.height*sizeof(uint32_t);
}

void ramdisk_read(void*,uint32_t,uint32_t);
void ramdisk_write(const void*,uint32_t,uint32_t);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len);

size_t fs_filesz(int fd)
{
  return file_table[fd].size;
}
//The file descriptor returned:the lowest-numbered file descriptor  not  currently open for the process.
int fs_open(const char *pathname,int flags,int mode)
{
  int i;
  for(i=0;i<NR_FILES;i++)
  {
    if(strcmp(pathname,file_table[i].name)==0)
    {
      file_table[i].open_offset=0;
      return i;
    }
  }
  panic("No such file named %s",pathname);
  //简易文件系统中每一个文件都是固定的,不会产生新文件,没有找到属于异常情况,使用 assertion 终止程序运行. 
  //panic does not check anything while assert does, in C, assert() only aborts execution when in debug mode.
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len)
{
  //stdin,stdout和stderr的操作直接忽略.后面三个特殊文件,3.2中可以忽略.
  assert(fd>2);
  if(fd==FD_EVENTS)
    return events_read(buf,len);
  //偏移量不要越过文件的边界
  int remain_bytes=file_table[fd].size-file_table[fd].open_offset;
  int bytes_to_read=(remain_bytes>len?len:remain_bytes);
  if(fd==FD_DISPINFO)
    dispinfo_read(buf,file_table[fd].disk_offset+file_table[fd].open_offset,bytes_to_read);
  else
    ramdisk_read(buf,file_table[fd].disk_offset+file_table[fd].open_offset,bytes_to_read);
  //the number of bytes read is returned
  file_table[fd].open_offset+=bytes_to_read;
  return bytes_to_read;
}

ssize_t fs_write(int fd, const void *buf, size_t len)
{
  Finfo *f=file_table+fd;
  //偏移量不要越过文件的边界
  int remain_bytes=f->size-f->open_offset;
  int bytes_to_write=(remain_bytes>len?len:remain_bytes);
  switch(fd)
  {
    case FD_STDOUT:
    case FD_STDERR:
      for(int i=0;i<len;i++)
        _putc(((char*)buf)[i]);
      break;
    case FD_EVENTS:
      return len;
    case FD_FB:
      //3.3中
      fb_write(buf,f->disk_offset+f->open_offset,bytes_to_write);
      break;
    default:
      ramdisk_write(buf,f->disk_offset+f->open_offset,bytes_to_write);
      break;
  }
  f->open_offset+=bytes_to_write;
  return bytes_to_write;
}

int fs_close(int fd)
{
  //简易文件系统没有维护文件打开的状态,直接返回 0,表示总是关闭成功
  return 0;
}

off_t fs_lseek(int fd, off_t offset, int whence)
{//repositions the offset of the open file fd to the argument offset according to the directive whence
  Finfo *f=file_table+fd;
  int new_offset=f->open_offset;
  int file_size=file_table[fd].size;
  switch(whence)
  {
    case SEEK_CUR:
      //The offset is set to its current location plus offset bytes.
      new_offset+=offset;
      break;
    case SEEK_SET:
      //The offset is set to offset bytes.
      new_offset=offset;
      break;
    case SEEK_END:
      //The offset is set to the size of the file plus offset bytes.
      new_offset=file_size+offset;
      break;
    default:
      assert(0);
  }
  if(new_offset<0)
    new_offset=0;
  else if(new_offset>file_size)
    new_offset=file_size;
  f->open_offset=new_offset;
  return new_offset;
}
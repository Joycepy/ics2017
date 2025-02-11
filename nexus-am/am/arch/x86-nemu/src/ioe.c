#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  //return 0;
  //访问RTC寄存器获得当前时间
  return inl(RTC_PORT)-boot_time;
}
//VGA初始化时注册了从0x40000开始的一段内存
uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);
//将pixels指定的矩形像素绘制到以`(x, y)`和`(x+w, y+h)`连线为对角线的矩形
void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  /*int i;
  for (i = 0; i < _screen.width * _screen.height; i++) {
    fb[i] = i;
  }*/
  //---mycode---
  int cp_bytes=sizeof(uint32_t) * (w<_screen.width-x?w:_screen.width-x);
  for(int j=0;j<h && y+j<_screen.height;j++)
  {
	memcpy(&fb[(y+j)*_screen.width+x],pixels,cp_bytes);
	pixels+=w;
  }
}

void _draw_sync() {
}

int _read_key() {
  if(inb(0x64)&0x1)
	  return inl(0x60);
  return _KEY_NONE;
}

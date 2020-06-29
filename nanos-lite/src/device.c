#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  //把事件写入到 buf 中,最长写入 len 字节, 返回写入的实际长度
  //借助 IOE 的 API 获得设备的输入
  //时钟事件可以任意时刻读取,优先处理按键事件
  int key=_read_key();
  //按下:键盘发送通码 释放:键盘发送断码,通码=断码+0x8000
  char keydown_char=(key&0x8000?'d':'u');
  int keyid=key & ~0x8000;
  if(keyid!=_KEY_NONE)
  {
    snprintf(buf,len,"k%c %s\n",keydown_char,keyname[keyid]);
    if ((key & 0x8000)&&(keyid == _KEY_F12))
    {//释放f12键
      extern void switch_game();
      switch_game();
    }
    return strlen(buf);
  }
  else
  {
    unsigned long time_ms=_uptime();
    return snprintf(buf,len,"t %d\n",time_ms)-1;
  }
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf,dispinfo+offset,len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  offset /= sizeof(uint32_t);
  //nexus-am/am/arch/x86-nemu/src/ioe.c
  //把 buf 中的 len 字节写到屏幕上 offset 处
  _draw_rect(buf,offset%_screen.width,offset/_screen.width,len/sizeof(uint32_t),1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo,"WIDTH: %d\nHEIGHT: %d",_screen.width,_screen.height);
}

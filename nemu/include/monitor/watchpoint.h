#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[128];
  uint32_t old_value;
  
} WP;

void delete_wp(int t);
void list_wp();
void set_wp(char* expression);
bool check_wp();
#endif

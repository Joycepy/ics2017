#include "common.h"
_RegSet* do_syscall(_RegSet *r);
_RegSet* schedule(_RegSet *prev);
static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:
      do_syscall(r);break;
      //return schedule(do_syscall(r));
    case _EVENT_TRAP:
      //printf("_EVENT_TRAP\n");
      return schedule(r);
    case _EVENT_IRQ_TIME: 
       //Log("Received _EVENT_IRQ_TIME");
      return schedule(r);
    default: panic("Unhandled event ID = %d", e.event);
  }
  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}

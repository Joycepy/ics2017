#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

//---MyCode---
static int cmd_si(char *args){
	int t=1;
	char *s=strtok(NULL," ");
	if(s!=NULL){
		t=atoi(s);
	}
	if(t>0)
		cpu_exec(t);
	else
		printf("Bad step number: %s\n",s);
	return 0;
}

static int cmd_info(char *args){
	char *s=strtok(NULL," ");
	if(s!=NULL){
		char t=s[0];
		if(t=='r'){
			//输出所有寄存器的值:isa_reg_display();
			int i;
			for(i=0;i<8;i++)
				printf("%s: 0x%08x\n",regsl[i],cpu.gpr[i]._32);
			for(i=0;i<8;i++)
				printf("%s: 0x%04x\n",regsw[i],cpu.gpr[i]._16);
			for(i=0;i<4;i++){
				printf("%s: 0x%02x\n",regsb[i*2],cpu.gpr[i]._8[0]);
				printf("%s: 0x%02x\n",regsb[i*2+1],cpu.gpr[i]._8[1]);
			}
			printf("eip: 0x%08x\n",cpu.eip);
		}else if(t=='w'){
		//打印监视点
			list_wp();		
		}
		else
			printf("Undefined info args.\n");
	}
	return 0;
}

static int cmd_x(char *args){//扫描内存
	int N=atoi(strtok(NULL," "));
	if(N>0){
		char*expression;
		expression=strtok(NULL," ");
		if(!expression){
			printf("Bad x args.\n");
			return 0;
		}
		bool success;
		vaddr_t location=expr(expression,&success);
		if(!success){
			printf("Invalid expression.\n");
			return 0;
		}
		//printf("Content at esp: %d\n",vaddr_read(0x6dbf2132,4));
		while(N>0){
			printf("Addr:0x%08x: \t Content: ",location);
			vaddr_t content=vaddr_read(location,4);
			printf("0x%08x\n",content);
			location+=4;
			N--;
		}
		return 0;
	}
	else
		printf("Bad x args.\n");
	return 0;
}

static int cmd_d(char *args){//删除监视点
	int t=0;
	char *s=strtok(NULL," ");
	if(s){
		t=atoi(s);
		delete_wp(t);
	}
	else
		printf("Bad delete args.\n");
	return 0;
}

static int cmd_w(char *args){//设置监视点
	char *expression=strtok(NULL," ");
	if(expression){
		set_wp(expression);
	}
	return 0;
}
static int cmd_p(char *args){//计算表达式
	//char *expression=strtok(NULL," ");
	char *expression=args;
	if(expression){
		bool success;
		int res=expr(expression,&success);
		if(success)
			printf("Result: %d\n",res);
	}
	return 0;
}
//---finish---
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Pause the program after N(default=1)-step execution",cmd_si},
  {"info", "Print program status", cmd_info},
  {"x", "Scan memory", cmd_x},
  {"d","Free the watchpoint whose NO=n",cmd_d},
  {"w","Set watchpoint",cmd_w},
  {"p","",cmd_p},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD){printf("Unknown command '%s'\n", cmd);}
  }
}

#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ,TK_NUM,TK_HEX,TK_REG,TK_SYMB,//标识符
	TK_NG,TK_NL,TK_AND,TK_OR,TK_REF,//指针
	TK_NEG//负数
  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal

  //---MyCode---
  
  {"\\*", '*'},         // multiply
  {"/", '/'},         // dividek
  {"%",'%'},
  {"\\-", '-'},         // minus
  {"\\(", '('},         // LPar
  {"\\)", ')'},         // RPar
  {"0[x,X][0-9a-fA-F]+",TK_HEX},       //hexadecimal integer
  {"[0-9]+",TK_NUM},       //decimal integer 
  {"\\!=", TK_NEQ},         // not equal
  {"\\$(e[e,d]i|e[s,i,b]p|e[a,b,c,d]x)",TK_REG}, //register暂时只考虑9个32位寄存器
  {"[a-zA-Z_][a-zA-Z_0-9]*",TK_SYMB},
  {"<=",TK_NG},
  {">=",TK_NL},
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"<",'<'},
  {">",'>'},
  {"&",'&'},
  {"\\|",'|'},
  {"!",'!'},
  {"\\^",'^'},
  {"~",'~'},

  //---finifsh---

};


#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[128];//改掉32，否则长表达式放不下
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;//准备判断下一个单词

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		int j=0;
        switch (rules[i].token_type) {
			case TK_NOTYPE:
				break;
			case TK_NUM:
			case TK_REG:
			case TK_HEX:
			case TK_SYMB:
				for(j=0;j<substr_len;j++)
					tokens[nr_token].str[j]=*(substr_start+j);
				tokens[nr_token].str[j]=0;
				tokens[nr_token].type=rules[i].token_type;
				nr_token++;
				break;
			//'+','-','*','/','%','(',')','<','>','!','~','&','|','^'
			//TK_EQ,TK_AND,TK_OR,TK_NEQ,TK_NG,TK_NL:
			default:
				tokens[nr_token].type=rules[i].token_type;
				nr_token++;
		}
        break;//匹配上了一个就可退出for循环
      }
    }

    if (i == NR_REGEX) {//一个也没匹配上
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
//---MyCode---
bool isOperand(int type){
	if(type!=TK_SYMB&&type!=TK_NUM&&type!=TK_HEX&&type!=TK_REG&&type!='('&&type!=')')
		return true;
	    return false;
}

bool check_parentheses(int p,int q,bool*success){
	if(tokens[p].type!='('||tokens[q].type!=')')return false;
	int tp=p;int rcount=0;
	for(tp=p;tp<q;tp++){
		if(tokens[tp].type=='(')rcount++;
		else if(tokens[tp].type==')')rcount--;
		if(rcount==0)return false;
	}
	if(tokens[q].type==')'){
		rcount--;
		if(rcount==0)
			return true;
	}
	if(rcount!=0){
		//括号不匹配
		*success=false;
		printf("unmatched parentheses.\n");
		assert(0);
		//return true;
	}
	return false;
}

static struct Operand{
	int type;
	int priority;
}table[]={
	{TK_NEG,2},//一元运算
	{TK_REF,2},
	{'!',2},
	{'%',3},//算术运算
	{'/',3},
	{'*',3},
	{'+',4},
	{'-',4},//未定义左移右移运算
	{TK_NG,5},//关系运算
	{TK_NL,5},
	{'<',5},
	{'>',5},
	{TK_NEQ,6},
	{TK_EQ,6},
	{'&',7},//位运算
	{'^',8},
	{'|',9},
	{TK_AND,10},//逻辑运算
	{TK_OR,11},
};

#define NR_TABLE (sizeof(table)/sizeof(table[0]))

uint32_t eval(int p,int q,bool*success){
	if(p>q){
		*success = false;
		return 0;
	}
	else if(p==q){
		uint32_t res=0;
		switch(tokens[p].type){
			case TK_NUM:
				sscanf(tokens[p].str,"%d",&res);//要使用res的引用
				return res;
			case TK_HEX:
				sscanf(tokens[p].str,"%x",&res);//%x读入16进制整数
				return res;
			case TK_REG:
				if(tokens[p].str[2]=='a')return cpu.eax;//rtlreg_t is exactly uint32_t
				if(tokens[p].str[2]=='c')return cpu.ecx;
				if(tokens[p].str[2]=='b'){
					if(tokens[p].str[3]=='x')return cpu.ebx;
					else return cpu.ebp;
				}
				if(tokens[p].str[2]=='d'){
					if(tokens[p].str[3]=='x')return cpu.edx;
					else return cpu.edi;
				}
				if(tokens[p].str[2]=='i')return cpu.eip;
				if(tokens[p].str[3]=='p')return cpu.esp;
				if(tokens[p].str[3]=='i')return cpu.esi;
			case TK_SYMB://需要符号表辅助，未完成
				break;
			default:
				*success=false;
		}
		printf("p == q and the result is %d\n",res);
		return res;
	}
	else if(check_parentheses(p,q,success)){
		/*if(*success==false){//括号不匹配
			printf("unmatched parentheses.\n");
			return 0;
		}*/
		return eval(p+1,q-1,success);	
	}
	else{
		//运算符和优先级太多，需要将符号对应到数字
		int op=-1;//the position of 主运算符
		int op_priority=0;
		int i;
		for(i=p;i<=q;i++){
			if(tokens[i].type=='('){
				//注意不是与第一个遇到的右括号匹配
				int k=1;
				while(k!=0){
					i++;
					if(tokens[i].type=='(')k++;
					else if(tokens[i].type==')')k--;
				}
			//	printf("find right one at %d\n",i);
			}
			else if(tokens[i].type==')'){
				assert(0);
			}
			else if(isOperand(tokens[i].type)){
				//优先级
				int j;
				for(j=0;i<NR_TABLE;j++){
					if(table[j].type==tokens[i].type)
						break;
				}
				if(table[j].priority>=op_priority){//=不适用于TK_REF和！（右结合）细节暂不处理
					op_priority=table[j].priority;
					op=i;
				}
			//	printf("----------------at %d :operand:%c\n",op,tokens[i].type);
			}
		}

		int op_type=tokens[op].type;

		//一元运算没有val1
		uint32_t val1=0;
		if(op_type!=TK_NEG&&op_type!=TK_REF&&op_type!='!')
			val1 = eval(p, op - 1,success);
		uint32_t val2 = eval(op + 1, q,success);
		switch (op_type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case '%': return val1 % val2;
			case TK_AND: return val1 && val2;
			case TK_OR: return val1 || val2;
			case TK_NG: return val1 <= val2;
			case TK_NL: return val1 >= val2;
			case '<': return val1 < val2;
			case '>': return val1 > val2;
			case TK_EQ: return val1 == val2;
			case TK_NEQ: return val1 != val2;
			case '^': return val1 ^ val2;
			case '&': return val1 & val2;
			case '|': return val1 | val2;
			case '!': return !val2;
			case TK_REF: return vaddr_read(val2,4);//读取内存:偏移地址，字节数
			case TK_NEG: return -val2;
			default: assert(0);
		}
	}
}
//---finish---
uint32_t expr(char *e, bool *success) {
  *success=true;
  if (!make_token(e)) {
	  printf("Error in Function-make_token.\n");
	  *success = false;
	  return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //在这里对*和-的类型重新处理,如果无前驱或前驱为操作符，则改变
  int i;
  for(i=0;i<nr_token;i++){
  if(tokens[i].type=='*'&&(i==0||isOperand(tokens[i-1].type)))
	  tokens[i].type=TK_REF;
  else if(tokens[i].type=='-'&&(i==0||isOperand(tokens[i-1].type)))
	  tokens[i].type=TK_NEG;
  }
  return eval(0,nr_token-1,success);//之前误写成了*success
}

#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
//---MyCode---

static WP* new_wp(){
	if(!free_)assert(0);//没有空闲的
	WP* res=free_;
	free_=free_->next;
	return res;
}

static void free_wp(WP *wp){
	wp->next=free_;
	free_=wp;
}

//根据序号删除
void delete_wp(int NO){
	WP *p,*pre;
	pre=NULL;
	for(p=head;p!=NULL;pre=p,p=p->next){
		if(p->NO==NO){
			break;
		}
	}
	if(p==NULL)return;
	if(pre==NULL)head=p->next;
	else pre->next=p->next;
	free_wp(p);
}

void list_wp(){
	if(head==NULL){
		printf("No watchpoint.\n");
		return;
	}
	WP *tmp;
	printf("\tNO\tValue\tExpression\n");
	for(tmp=head;tmp!=NULL;tmp=tmp->next){
		printf("\t%d\t%d\t%s\n",tmp->NO,tmp->old_value,tmp->expr);
	}
}

void set_wp(char* expression){
	WP *tmp=new_wp();
	strcpy(tmp->expr,expression);
	bool success;
	tmp->old_value=expr(expression,&success);
	tmp->next=head;
	head=tmp;
}

bool check_wp(){//检查监视点是否变化
	if(head==NULL)return true;
	WP *tmp=head;
	while(tmp!=NULL){
		bool success;
		uint32_t new_value=expr(tmp->expr,&success);
		if(success){
			if(new_value!=tmp->old_value){
				tmp->old_value=new_value;
				printf("\tNO\tValue\tExpression\n");
				printf("\t%d\t%d\t%s\n",tmp->NO,tmp->old_value,tmp->expr);
				return false;
			}
		}
	}
	return true;
}
//---finish---

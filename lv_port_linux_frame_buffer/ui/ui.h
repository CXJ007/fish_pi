#ifndef UI_H
#define UI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

extern struct page_list *page_head;
extern struct cmd_list *cmd_head;
extern pthread_mutex_t lvgl_mutex;


struct page_list{
    char name[10];
    lv_obj_t *obj;
    lv_obj_t* (*create)(void);
    void (*delete)(lv_obj_t* obj);
    struct page_list *next;
};


union cmd{
        char cmdbuf[20];
        int cmd;
};
struct cmd_list{
    char name[10];
    union cmd data;
    struct cmd_list *next;
};


void *lvgl_start(void *arg);
void *cmd_handle(void *arg);

void ui_init(void);
void page_add(struct page_list *head,char *name,lv_obj_t* (*create)(void), void (*delete)(lv_obj_t* ));
void page_create(struct page_list *head, char* name);

void home_page_add(struct page_list *head);
void tag_page_add(struct page_list *head);
void menu_page_add(struct page_list *head);
void time_page_add(struct page_list *head);

lv_obj_t *obj_read(struct page_list *head, char* name);

void cmd_add(struct cmd_list *head,char *name);
void cmd_read(struct cmd_list *head, char *name, union cmd *data,int flag);
void cmd_write(struct cmd_list *head, char *name, union cmd data);
void home_cmd_add(struct cmd_list *head);
void home_cmd_read(struct cmd_list *head, union cmd *data);
void menu_cmd_add(struct cmd_list *head);
void switch_cmd_write(struct cmd_list *head, union cmd data);
void switch_cmd_read(struct cmd_list *head, union cmd *data);
void time_cmd_add(struct cmd_list *head);
void time_cmd_write(struct cmd_list *head, union cmd data);
void time_cmd_read(struct cmd_list *head, union cmd *data);



#endif
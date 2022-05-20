#ifndef UI_H
#define UI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
//#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

extern struct page_list *page_head;
extern struct cmd_list *cmd_head;
extern pthread_mutex_t lvgl_mutex;
extern uint32_t MY_EVENT_TIME;
extern char sys_date[50]; 

struct page_list{
    char name[10];
    lv_obj_t *obj;
    int flag;
    lv_obj_t* (*create)(void);
    void (*delete)(lv_obj_t* obj);
    struct page_list *next;
};



struct cmd_data{
    union{
        char name[20];
        int cmd;
    }cmd_name;
    union{
        char info[50];
    }cmd_info;
};

struct cmd_list{
    char name[10];
    struct cmd_data cmd;
    struct cmd_list *next;
};


void *lvgl_start(void *arg);
void *cmd_handle(void *arg);
void mutex_init(void);

void ui_init(void);
void page_add(struct page_list *head,char *name,lv_obj_t* (*create)(void), void (*delete)(lv_obj_t* ));
void page_create(struct page_list *head, char* name);
void page_delete(struct page_list *head, char* name);
int page_check(struct page_list *head, char* name);
void page_flag_set(struct page_list *head, char* name, int flag);

void home_page_add(struct page_list *head);
void tag_page_add(struct page_list *head);
void menu_page_add(struct page_list *head);
void time_page_add(struct page_list *head);
void music_page_add(struct page_list *head);

lv_obj_t *obj_read(struct page_list *head, char* name);

void cmd_add(struct cmd_list *head,char *name);
void cmd_read(struct cmd_list *head, char *name, struct cmd_data *cmd,int flag);
void cmd_write(struct cmd_list *head, char *name, struct cmd_data cmd);
void home_cmd_add(struct cmd_list *head);
void home_cmd_write(struct cmd_list *head, struct cmd_data cmd);
void home_cmd_read(struct cmd_list *head, struct cmd_data *cmd);
void menu_cmd_add(struct cmd_list *head);
void switch_cmd_write(struct cmd_list *head, struct cmd_data cmd);
void switch_cmd_read(struct cmd_list *head, struct cmd_data *cmd);
void time_cmd_add(struct cmd_list *head);
void time_cmd_write(struct cmd_list *head, struct cmd_data cmd);
void time_cmd_read(struct cmd_list *head, struct cmd_data *cmd);
void tag_cmd_add(struct cmd_list *head);
void tag_cmd_write(struct cmd_list *head, struct cmd_data cmd);
void tag_cmd_read(struct cmd_list *head, struct cmd_data *cmd);
void music_cmd_add(struct cmd_list *head);
void music_cmd_write(struct cmd_list *head, struct cmd_data cmd);
void music_cmd_read(struct cmd_list *head, struct cmd_data *cmd);

void home_cmd_handle(void);
void switch_cmd_handle(void);
void time_cmd_handle(void);
void tag_cmd_handle(void);
void music_cmd_handle(void);


#endif
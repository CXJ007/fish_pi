#include "ui.h"

struct page_list *page_head;
struct cmd_list *cmd_head;
pthread_mutex_t lvgl_mutex;
pthread_mutex_t cmd_mutex;
pthread_mutex_t page_mutex;
pthread_cond_t  cmd_cond;
uint32_t MY_EVENT_TIME;
char new_cmd_name[10];

static lv_timer_t * lvgl_timer;
static int h=0, m=0, s=0;
static int sync_home, sync_tag;
//date +%Y-%m-%d::%T

void page_add(struct page_list *head,char *name,lv_obj_t* (*create)(void), void (*delete)(lv_obj_t* ))
{
    pthread_mutex_lock(&page_mutex);
    if(head == NULL){
        printf("head err\n");
        pthread_mutex_unlock(&page_mutex);
        return ;
    }
    struct page_list *node = malloc(sizeof(struct page_list));
    strcpy(node->name, name);
    node->flag = 0;
    node->create = create;
    node->delete = delete;
    node->next = head->next;
    head->next = node;
    pthread_mutex_unlock(&page_mutex);   
}

void page_create(struct page_list *head, char* name)
{
    pthread_mutex_lock(&page_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            head->obj = head->create();
            head->flag = 1;
            break;
        }
        head = head->next;
    }
    pthread_mutex_unlock(&page_mutex);
}

void page_delete(struct page_list *head, char* name)
{
    pthread_mutex_lock(&page_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            head->flag = 0;
            head->delete(head->obj);
            break;
        }
        head = head->next;
    }
    pthread_mutex_unlock(&page_mutex);
}

int page_check(struct page_list *head, char* name)
{
    pthread_mutex_lock(&page_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            pthread_mutex_unlock(&page_mutex);
            return head->flag;
        }
        head = head->next;
    }
    pthread_mutex_unlock(&page_mutex);
    return 0;
}

void page_flag_set(struct page_list *head, char* name, int flag)
{
    pthread_mutex_lock(&page_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            head->flag = flag;
            break;
        }
        head = head->next;
    }
    pthread_mutex_unlock(&page_mutex);
}

void cmd_mutex_init(void)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lvgl_mutex,  &attr);
    pthread_mutex_init(&page_mutex, &attr);
    pthread_mutex_init(&cmd_mutex,  &attr);
    pthread_cond_init(&cmd_cond, NULL);
}

void cmd_add(struct cmd_list *head, char *name)
{
    pthread_mutex_lock(&cmd_mutex);
    if(head == NULL){
        printf("head err\n");
        return ;
    }
    struct cmd_list *node = malloc(sizeof(struct cmd_list));
    strcpy(node->name, name);
    node->next = head->next;
    head->next = node; 
    pthread_mutex_unlock(&cmd_mutex);   
}

void cmd_read(struct cmd_list *head, char *name, struct cmd_data *cmd,int flag)
{
    pthread_mutex_lock(&cmd_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            *cmd = head->cmd;
            if(flag == 1){
                memset(&head->cmd.cmd_info, '0', 50);
            }
            break;
        }
        head = head->next;
    }
    pthread_mutex_unlock(&cmd_mutex);
}

void cmd_write(struct cmd_list *head, char *name, struct cmd_data cmd)
{
    pthread_mutex_lock(&cmd_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            head->cmd = cmd;
            strcpy(new_cmd_name, name);
            break;
        }
        head = head->next;
    }
    pthread_mutex_unlock(&cmd_mutex);
    pthread_cond_signal(&cmd_cond);
}

lv_obj_t *obj_read(struct page_list *head, char* name)
{
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            return head->obj;
        }
        head = head->next;
    }
    return (lv_obj_t *)NULL;
}

static void sys_event_cb(lv_event_t * e)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_event_code_t code = lv_event_get_code(e);
    pthread_mutex_unlock(&lvgl_mutex);

    if(code == MY_EVENT_TIME){
        if(sync_tag == 1){
            if(page_check(page_head, "tag") == 1){
                sync_tag = 0;
                struct cmd_data cmd;
                strcpy(cmd.cmd_name.name, "time sync");
                sprintf(cmd.cmd_info.info, "%d%d:%d%d:%d%d", h/10, h%10, m/10, m%10, s/10, s%10);
                tag_cmd_write(cmd_head, cmd);
            }
        }
        if(sync_home == 1){
            if(page_check(page_head, "home") == 1){
                sync_home = 0;
                struct cmd_data cmd;
                strcpy(cmd.cmd_name.name, "time sync");
                sprintf(cmd.cmd_info.info, "%d:%d", h, m);
                home_cmd_write(cmd_head, cmd);
            }
        }
        //printf("%d  %d  %d\n", h, m, s);
    }

}

static void sys_time_sync(lv_timer_t * timer)
{
    s++;
    sync_tag = 1;
    if(s >= 60){
        m++;
        s = 0;
        sync_home = 1;
    }
    if(m >= 60){
        h++;
        m = 0;
    }
    if(h >= 24){
        h = 0;
    }
    lv_event_send(lv_scr_act(), MY_EVENT_TIME, NULL);
    //printf("%d-%d-%d\n", h,m,s);
}

void sys_time_init(void)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_add_event_cb(lv_scr_act(), sys_event_cb, MY_EVENT_TIME , NULL);
    lvgl_timer = lv_timer_create(sys_time_sync, 300,  NULL);
    pthread_mutex_unlock(&lvgl_mutex);
}

void ui_init(void)
{
    page_head = malloc(sizeof(struct page_list));
    page_head = (struct page_list *)memset((void*)page_head, 0, sizeof(struct page_list));
    cmd_head = malloc(sizeof(struct cmd_list));
    cmd_head = (struct cmd_list *)memset((void*)cmd_head, 0, sizeof(struct cmd_list));

    home_page_add(page_head);
    tag_page_add(page_head);
    menu_page_add(page_head);
    time_page_add(page_head);

    home_cmd_add(cmd_head);
    menu_cmd_add(cmd_head);
    tag_cmd_add(cmd_head);
    
    page_create(page_head, "home"); 

    MY_EVENT_TIME = lv_event_register_id();

    sys_time_init();
}


#define DISP_BUF_SIZE (240 * 240)

void *lvgl_start(void *arg)
{
     /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 240;
    disp_drv.ver_res    = 240;
    lv_disp_drv_register(&disp_drv);

    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = evdev_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);


    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/


    /*Create a Demo*/
    ui_init();
    
    

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        pthread_mutex_lock(&lvgl_mutex);
        lv_timer_handler();
        pthread_mutex_unlock(&lvgl_mutex);
        usleep(5000);
    }

    pthread_exit((void *)0);
}

void *cmd_handle(void *arg)
{
    struct cmd_data cmd;
    while(1){
        pthread_mutex_lock(&cmd_mutex);
        pthread_cond_wait(&cmd_cond, &cmd_mutex);
        pthread_mutex_unlock(&cmd_mutex);
        //printf("%s\n", new_cmd_name);
        if(strcmp(new_cmd_name, "home") == 0){
            home_cmd_handle();
        }else if(strcmp(new_cmd_name, "switch") == 0){
            switch_cmd_handle();
        }else if(strcmp(new_cmd_name, "time") == 0){
            time_cmd_handle();
        }else if(strcmp(new_cmd_name, "tag") == 0){
            tag_cmd_handle();
        }
        //memset(data.cmdbuf, '/0', 20);
    }

    pthread_exit((void *)0);
}

#include "ui.h"

struct page_list *page_head;
struct cmd_list *cmd_head;
pthread_mutex_t lvgl_mutex;
pthread_mutex_t cmd_mutex;
pthread_cond_t  cmd_cond;
char new_cmd_name[10];

void page_add(struct page_list *head,char *name,lv_obj_t* (*create)(void), void (*delete)(lv_obj_t* ))
{
    if(head == NULL){
        printf("head err\n");
        return ;
    }
    struct page_list *node = malloc(sizeof(struct page_list));
    strcpy(node->name, name);
    node->create = create;
    node->delete = delete;
    node->next = head->next;
    head->next = node;    
}

void page_create(struct page_list *head, char* name)
{
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            head->obj = head->create();
            break;
        }
        head = head->next;
    }
}

void page_delete(struct page_list *head, char* name)
{
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            head->delete(head->obj);
            break;
        }
        head = head->next;
    }
}

void cmd_mutex_init(void)
{
    pthread_mutex_init(&lvgl_mutex, NULL);
    pthread_mutex_init(&cmd_mutex, NULL);
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

void cmd_read(struct cmd_list *head, char *name, union cmd *data,int flag)
{
    pthread_mutex_lock(&cmd_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            *data = head->data;
            if(flag == 1){
                memset(&head->data, '/0', 20);
            }
            break;
        }
        head = head->next;
    }
    pthread_mutex_unlock(&cmd_mutex);
}

void cmd_write(struct cmd_list *head, char *name, union cmd data)
{
    pthread_mutex_lock(&cmd_mutex);
    head = head->next;
    while(head != NULL){
        if(strcmp(head->name, name) == 0){
            head->data = data;
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

    page_create(page_head, "home");   
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
    union cmd data;
    while(1){
        pthread_mutex_lock(&cmd_mutex);
        pthread_cond_wait(&cmd_cond, &cmd_mutex);
        pthread_mutex_unlock(&cmd_mutex);
        printf("%s\n", new_cmd_name);
        if(strcmp(new_cmd_name, "home") == 0){
            home_cmd_read(cmd_head, &data);
            if(strcmp(data.cmdbuf,"menu create") == 0){
                page_create(page_head, "menu");
                page_create(page_head, "tag");
            }
        }else if(strcmp(new_cmd_name, "switch") == 0){
            switch_cmd_read(cmd_head, &data);
            if(strcmp(data.cmdbuf,"home create") == 0){
                page_delete(page_head, "menu");
                page_create(page_head, "home");
                
            }
        }else if(strcmp(new_cmd_name, "time") == 0){
            time_cmd_read(cmd_head, &data);
            if(strcmp(data.cmdbuf,"time create") == 0){
                page_create(page_head, "time"); 
            }
        }
        //memset(data.cmdbuf, '/0', 20);
    }

    pthread_exit((void *)0);
}

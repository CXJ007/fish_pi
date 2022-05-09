#include "ui.h"
#include "anim_num/anim_num.h"


static struct anim_num* num0;
static struct anim_num* num1;
static struct anim_num* num2;
static struct anim_num* num3;

static lv_timer_t * timer;
static int h,m;

void delete_home_timer(lv_timer_t * timer)
{
    struct cmd_data cmd;
    strcpy(cmd.cmd_name.name, "menu create");
    lv_obj_t *home = (lv_obj_t *)timer->user_data;
    home_cmd_write(cmd_head, cmd);
}

static void anim_y_cb(void * var, int32_t v)
{
    lv_obj_set_y(var, v);
}

static void home_even_cb(lv_event_t *e)
{
    lv_obj_t *home = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_anim_t a;
    if(code == LV_EVENT_PRESSED){
        lv_anim_init(&a);
        lv_anim_set_var(&a, home);
        lv_anim_set_values(&a, lv_obj_get_y(home), -240);
        lv_anim_set_time(&a, 500);
        lv_anim_set_exec_cb(&a, anim_y_cb);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
        timer = lv_timer_create(delete_home_timer, 500,  home);
        lv_timer_set_repeat_count(timer, 1);
    }

}

static lv_obj_t* home_create(void)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);
    static lv_style_t style_obj;
    lv_style_init(&style_obj);
    lv_style_set_bg_color(&style_obj, lv_color_hex(0xc0c0c0));
    lv_style_set_radius(&style_obj, 0);
    lv_style_set_border_width(&style_obj, 0);
    lv_obj_t *home = lv_obj_create(lv_scr_act());
    lv_obj_set_size(home, 240, 240);
    lv_obj_add_style(home, &style_obj, LV_STATE_DEFAULT); 
    lv_obj_add_event_cb(home, home_even_cb, LV_EVENT_PRESSED , NULL);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, home);
    lv_anim_set_values(&a, -240, 0);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);


    
    num0 = anim_num_create(home,0,0,11,lv_color_hex(0x3e3d39),8);
    num1 = anim_num_create(home,0,0,11,lv_color_hex(0x3e3d39),8);
    num2 = anim_num_create(home,0,0,11,lv_color_hex(0x3e3d39),8);
    num3 = anim_num_create(home,0,0,11,lv_color_hex(0x3e3d39),8);
    lv_obj_align(num0->obj, LV_ALIGN_CENTER, -85, 0);
    lv_obj_align(num1->obj, LV_ALIGN_CENTER, -35, 0);
    lv_obj_align(num2->obj, LV_ALIGN_CENTER, 35, 0);
    lv_obj_align(num3->obj, LV_ALIGN_CENTER, 85, 0);

    // if(mutex == 0){
    //     mutex = 1;
    //     anim_num_disp(num0, h/10 , 1000);
    //     anim_num_disp(num1, h%10 , 1000);
    //     anim_num_disp(num2, m/10 , 1000);
    //     anim_num_disp(num3, m%10 , 1000);
    //     mutex = 0;
    // }
        anim_num_disp(num0, h/10, 1000);
        anim_num_disp(num1, h%10, 1000);
        anim_num_disp(num2, m/10, 1000);
        anim_num_disp(num3, m%10, 1000);
    
        pthread_mutex_unlock(&lvgl_mutex);

    return home;
}

static void home_delete(lv_obj_t* home)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_del(home);
    pthread_mutex_unlock(&lvgl_mutex);
}

void home_page_add(struct page_list *head)
{
    page_add(head, "home", home_create, home_delete);
}

void home_cmd_write(struct cmd_list *head, struct cmd_data cmd)
{
    cmd_write(head, "home", cmd);
}

void home_cmd_read(struct cmd_list *head, struct cmd_data *cmd)
{
    cmd_read(cmd_head, "home", cmd, 1);
}

void home_cmd_add(struct cmd_list *head)
{
    cmd_add(head, "home");
}

void home_cmd_handle(void)
{
    
    struct cmd_data cmd;
    home_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"menu create") == 0){
        page_create(page_head, "menu");
        page_create(page_head, "tag");
        page_delete(page_head, "home");
    }else if(strcmp(cmd.cmd_name.name,"time sync") == 0){
        char *delim = ":";
        h = atoi(strtok(cmd.cmd_info.info, delim));
        m = atoi(strtok(NULL, delim));
        if(page_check(page_head, "home") == 1){
            printf("time sync\n");
            anim_num_disp(num0, h/10, 1000);
            anim_num_disp(num1, h%10, 1000);
            anim_num_disp(num2, m/10, 1000);
            anim_num_disp(num3, m%10, 1000);
        }
    }
}

#include "ui.h"

LV_IMG_DECLARE(image13);
LV_IMG_DECLARE(image14);

static lv_obj_t *btn0;
static lv_obj_t *btn1;


static void set_zoom(void * img, int32_t v)
{
    lv_img_set_zoom(img, v);
}

static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void time_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        LV_LOG_USER("Selected value: %s", buf);
    }else if(code == LV_EVENT_PRESSED){
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, set_zoom);
        lv_anim_set_values(&a, 300, 255);
        lv_anim_set_time(&a, 500);
        lv_anim_start(&a);
    }else if(code == LV_EVENT_SHORT_CLICKED){
        if(obj == btn0){
            printf("btn0\n");
        }else if(obj == btn1){
            printf("btn1\n");
        }
    }
}


lv_obj_t* time_create(void)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_t *menu = obj_read(page_head, "menu");

    lv_obj_t *time = lv_obj_create(menu);
    lv_obj_set_size(time, 190, 210);
    lv_obj_set_style_pad_bottom(time, 0, 0);
    lv_obj_align(time, LV_ALIGN_LEFT_MID, 0, 15);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, time);
    lv_anim_set_values(&a, 240, 50);
    lv_anim_set_time(&a, 500);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    static lv_style_t style_roller;
    lv_style_init(&style_roller);
    lv_style_set_text_color(&style_roller, lv_color_black());
    lv_style_set_bg_color(&style_roller, lv_color_white());
    lv_style_set_pad_all(&style_roller, 0);
    lv_style_set_text_font(&style_roller,&lv_font_montserrat_32);
    lv_style_set_width(&style_roller, 50);


    const char * opts_h = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
    int i;
    char buf[5];
    char opts_ms[256];
    memset(opts_ms, 0, sizeof(opts_ms));
    for(i=0;i<=58;i++){
        sprintf(buf, "%d\n", i);
        strcat(opts_ms, buf);
    }
    sprintf(buf, "%d", 59);
    strcat(opts_ms, buf);

    lv_obj_t * roller_h = lv_roller_create(time);
    lv_roller_set_options(roller_h, opts_h, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_h, 3);
    lv_obj_set_width(roller_h, 50);
    lv_obj_set_style_text_font(roller_h, &lv_font_montserrat_16, 0);
    lv_obj_add_style(roller_h, &style_roller, LV_PART_SELECTED);
    lv_obj_align(roller_h, LV_ALIGN_CENTER, -55, -10);
    lv_obj_add_event_cb(roller_h, time_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    //lv_roller_set_selected(roller_h, 5, LV_ANIM_OFF);

    lv_obj_t * roller_m = lv_roller_create(time);
    lv_roller_set_options(roller_m, opts_ms, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_m, 3);
    lv_obj_set_width(roller_m, 50);
    lv_obj_set_style_text_font(roller_m, &lv_font_montserrat_16, 0);
    lv_obj_add_style(roller_m, &style_roller, LV_PART_SELECTED);
    lv_obj_align(roller_m, LV_ALIGN_CENTER, 0, -10);
    lv_obj_add_event_cb(roller_m, time_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    //lv_roller_set_selected(roller_m, 5, LV_ANIM_OFF);

    lv_obj_t *roller_s = lv_roller_create(time);
    lv_roller_set_options(roller_s, opts_ms, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_s, 3);
    lv_obj_set_width(roller_s, 50);
    lv_obj_set_style_text_font(roller_s, &lv_font_montserrat_16, 0);
    lv_obj_add_style(roller_s, &style_roller, LV_PART_SELECTED);
    lv_obj_align(roller_s, LV_ALIGN_CENTER, 55, -10);
    lv_obj_add_event_cb(roller_s, time_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    //lv_roller_set_selected(roller_s, 5, LV_ANIM_OFF);

    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, 0);
    lv_style_set_shadow_width(&style_btn, 0);
    lv_style_set_border_width(&style_btn, 0);

    btn0 = lv_btn_create(time);
    lv_obj_set_size(btn0, 50, 50);
    lv_obj_align(btn0, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *img0 = lv_img_create(btn0);
    lv_img_set_src(img0, &image13);
    lv_obj_center(img0);
    lv_obj_add_style(btn0, &style_btn, 0);
    lv_obj_add_event_cb(btn0, time_event_cb, LV_EVENT_PRESSED, (void *)img0);
    lv_obj_add_event_cb(btn0, time_event_cb, LV_EVENT_SHORT_CLICKED, (void *)img0);

    btn1 = lv_btn_create(time);
    lv_obj_set_size(btn1, 50, 50);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_t *img1 = lv_img_create(btn1);
    lv_img_set_src(img1, &image14);
    lv_obj_center(img1);
    lv_obj_add_style(btn1, &style_btn, 0);
    lv_obj_add_event_cb(btn1, time_event_cb, LV_EVENT_PRESSED, (void *)img1);
    lv_obj_add_event_cb(btn1, time_event_cb, LV_EVENT_SHORT_CLICKED, (void *)img1);

    pthread_mutex_unlock(&lvgl_mutex);
    
    return time;
}

static void time_delete(lv_obj_t* time)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_del(time);
    pthread_mutex_unlock(&lvgl_mutex);
}

void time_page_add(struct page_list *head)
{
    page_add(head, "time", time_create, time_delete);
}

void time_cmd_add(struct cmd_list *head)
{
    cmd_add(head, "time");
}

void time_cmd_write(struct cmd_list *head, struct cmd_data cmd)
{
    cmd_write(head, "time", cmd);
}

void time_cmd_read(struct cmd_list *head, struct cmd_data *cmd)
{
    cmd_read(cmd_head, "time", cmd, 1);
}

void time_cmd_handle(void)
{
    struct cmd_data cmd;
    time_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"time create") == 0){
        page_create(page_head, "time"); 
    }

}
#include "ui.h"

//LV_IMG_DECLARE(image0);
LV_IMG_DECLARE(image1);
LV_IMG_DECLARE(image2);
LV_IMG_DECLARE(image3);
LV_IMG_DECLARE(image4);
LV_IMG_DECLARE(image5);
LV_IMG_DECLARE(image6);
LV_IMG_DECLARE(image7);
LV_IMG_DECLARE(image8);
LV_IMG_DECLARE(image9);
LV_IMG_DECLARE(image10);
LV_IMG_DECLARE(image11);


static lv_obj_t *btn0;
static lv_obj_t *btn1;

static void set_zoom(void * img, int32_t v)
{
    lv_img_set_zoom(img, v);
}

static void menu_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(e);
    static lv_obj_t *last_btn;

    if(code == LV_EVENT_PRESSED){
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, set_zoom);
        lv_anim_set_values(&a, 300, 255);
        lv_anim_set_time(&a, 500);
        lv_anim_start(&a);
    }else if(code == LV_EVENT_SHORT_CLICKED){
        if(btn == btn0){
            struct cmd_data cmd;
            strcpy(cmd.cmd_name.name, "home create");
            switch_cmd_write(cmd_head, cmd);
        }else if((btn==btn1) && (last_btn != btn)){
            struct cmd_data cmd;
            strcpy(cmd.cmd_name.name, "time create");
            time_cmd_write(cmd_head, cmd);
        }
        last_btn = btn;
    }
}

lv_obj_t* menu_creat(void)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_t *menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu, 240, 240);
    lv_obj_align(menu, LV_ALIGN_CENTER, 0, 0);
    static lv_style_t style_menu;
    lv_style_init(&style_menu);
    lv_style_set_border_width(&style_menu, 0);
    lv_style_set_radius(&style_menu, 0);
    lv_style_set_pad_all(&style_menu, 0);
    lv_obj_add_style(menu, &style_menu, 0);

    static lv_style_t style_cont;
    lv_style_init(&style_cont);
    lv_style_set_bg_color(&style_cont, lv_color_hex(0xc0c0c0));
    lv_style_set_border_width(&style_cont, 0);
    lv_style_set_radius(&style_cont, 0);
    lv_style_set_pad_all(&style_cont, 5);
    lv_style_set_pad_right(&style_cont, 0);
    lv_style_set_pad_left(&style_cont, 0);
    lv_obj_t *cont_col = lv_obj_create(menu);
    lv_obj_set_size(cont_col, 50, 210);
    lv_obj_align(cont_col, LV_ALIGN_LEFT_MID, 0, 15);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(cont_col, &style_cont, 0);
    lv_obj_set_scrollbar_mode(cont_col, LV_SCROLLBAR_MODE_OFF);
    
    
    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, 0);
    lv_style_set_shadow_width(&style_btn, 0);
    lv_style_set_border_width(&style_btn, 0);
    

    btn0 = lv_btn_create(cont_col);
    lv_obj_set_size(btn0, 50, 50);
    lv_obj_t *img1 = lv_img_create(btn0);
    lv_img_set_src(img1, &image1);
    lv_obj_center(img1);
    lv_obj_add_event_cb(btn0, menu_event_cb, LV_EVENT_PRESSED, (void *)img1);
    lv_obj_add_event_cb(btn0, menu_event_cb, LV_EVENT_SHORT_CLICKED, (void *)img1);
    lv_obj_add_style(btn0, &style_btn, 0);

    btn1 = lv_btn_create(cont_col);
    lv_obj_set_size(btn1, 50, 50);
    lv_obj_t *img2 = lv_img_create(btn1);
    lv_img_set_src(img2, &image2);
    lv_obj_center(img2);
    lv_obj_add_event_cb(btn1, menu_event_cb, LV_EVENT_PRESSED, (void *)img2);
    lv_obj_add_event_cb(btn1, menu_event_cb, LV_EVENT_SHORT_CLICKED, (void *)img2);
    lv_obj_add_style(btn1, &style_btn, 0);

    lv_obj_t *btn2 = lv_btn_create(cont_col);
    lv_obj_set_size(btn2, 50, 50);
    lv_obj_t *img3 = lv_img_create(btn2);
    lv_img_set_src(img3, &image3);
    lv_obj_center(img3);
    // struct usr_data *data3 = (struct usr_data *)malloc(sizeof(struct usr_data));
    // data3->pagenum = page11;
    // data3->obj = img3;
    // lv_obj_add_event_cb(btn2, page1_event_cb, LV_EVENT_PRESSED, (void *)data3);
    lv_obj_add_style(btn2, &style_btn, 0);

    lv_obj_t *btn3 = lv_btn_create(cont_col);
    lv_obj_set_size(btn3, 50, 50);
    lv_obj_t *img4 = lv_img_create(btn3);
    lv_img_set_src(img4, &image4);
    lv_obj_center(img4);
    // struct usr_data *data4 = (struct usr_data *)malloc(sizeof(struct usr_data));
    // data4->pagenum = page12;
    // data4->obj = img4;
    // lv_obj_add_event_cb(btn3, page1_event_cb, LV_EVENT_PRESSED, (void *)data4);
    lv_obj_add_style(btn3, &style_btn, 0);

    lv_obj_t *btn4 = lv_btn_create(cont_col);
    lv_obj_set_size(btn4, 50, 50);
    lv_obj_t *img5 = lv_img_create(btn4);
    lv_img_set_src(img5, &image5);
    lv_obj_center(img5);
    // struct usr_data *data5 = (struct usr_data *)malloc(sizeof(struct usr_data));
    // data5->pagenum = page13;
    // data5->obj = img5;
    // lv_obj_add_event_cb(btn4, page1_event_cb, LV_EVENT_PRESSED, (void *)data5);
    lv_obj_add_style(btn4, &style_btn, 0);

    lv_obj_t *btn5 = lv_btn_create(cont_col);
    lv_obj_set_size(btn5, 50, 50);
    lv_obj_t *img6 = lv_img_create(btn5);
    lv_img_set_src(img6, &image6);
    lv_obj_center(img6);
    // struct usr_data *data6 = (struct usr_data *)malloc(sizeof(struct usr_data));
    // data6->pagenum = page14;
    // data6->obj = img6;
    // lv_obj_add_event_cb(btn5, page1_event_cb, LV_EVENT_PRESSED, (void *)data6);
    lv_obj_add_style(btn5, &style_btn, 0);

    lv_obj_t *btn6 = lv_btn_create(cont_col);
    lv_obj_set_size(btn6, 50, 50);
    lv_obj_t *img7 = lv_img_create(btn6);
    lv_img_set_src(img7, &image7);
    lv_obj_center(img7);
    // struct usr_data *data7 = (struct usr_data *)malloc(sizeof(struct usr_data));
    // data7->pagenum = page15;
    // data7->obj = img7;
    // lv_obj_add_event_cb(btn6, page1_event_cb, LV_EVENT_PRESSED, (void *)data7);
    lv_obj_add_style(btn6, &style_btn, 0);

    lv_obj_t *btn7 = lv_btn_create(cont_col);
    lv_obj_set_size(btn7, 50, 50);
    lv_obj_t *img8 = lv_img_create(btn7);
    lv_img_set_src(img8, &image8);
    lv_obj_center(img8);
    // struct usr_data *data8 = (struct usr_data *)malloc(sizeof(struct usr_data));
    // data8->pagenum = page16;
    // data8->obj = img8;
    // lv_obj_add_event_cb(btn7, page1_event_cb, LV_EVENT_PRESSED, (void *)data8);
    lv_obj_add_style(btn7, &style_btn, 0);

    lv_obj_t *btn8 = lv_btn_create(cont_col);
    lv_obj_set_size(btn8, 50, 50);
    lv_obj_t *img9 = lv_img_create(btn8);
    lv_img_set_src(img9, &image9);
    lv_obj_center(img9);
    // struct usr_data *data9 = (struct usr_data *)malloc(sizeof(struct usr_data));
    // data9->pagenum = page17;
    // data9->obj = img9;
    // lv_obj_add_event_cb(btn8, page1_event_cb, LV_EVENT_PRESSED, (void *)data9);
    lv_obj_add_style(btn8, &style_btn, 0);
    pthread_mutex_unlock(&lvgl_mutex);
    
    return menu;
}

static void menu_delete(lv_obj_t* menu)
{
    page_flag_set(page_head, "tag", 0);
    page_flag_set(page_head, "time", 0);
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_del(menu);
    pthread_mutex_unlock(&lvgl_mutex);
}

void menu_page_add(struct page_list *head)
{
    page_add(head, "menu", menu_creat, menu_delete);
}

void menu_cmd_add(struct cmd_list *head)
{
    cmd_add(head, "switch");
    time_cmd_add(head);
}

void switch_cmd_write(struct cmd_list *head, struct cmd_data cmd)
{
    cmd_write(head, "switch", cmd);
}

void switch_cmd_read(struct cmd_list *head, struct cmd_data *cmd)
{
    cmd_read(cmd_head, "switch", cmd, 1);
}

void switch_cmd_handle(void)
{
    struct cmd_data cmd;
    switch_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"home create") == 0){
        page_delete(page_head, "menu");
        page_create(page_head, "home");
    }
}
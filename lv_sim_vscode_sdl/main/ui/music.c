#include "ui.h"

LV_IMG_DECLARE(image15);
LV_IMG_DECLARE(image16);
LV_IMG_DECLARE(image17);
LV_IMG_DECLARE(image18);
static lv_style_t style_btn;
static lv_style_t style_control;
static lv_style_t style_slider_main;
static lv_obj_t *btn0;
static lv_obj_t *btn1;
static lv_obj_t *btn2;
static lv_obj_t *slider_time;
static lv_obj_t *slider_voice;
static lv_obj_t *lable_start;
static lv_obj_t *lable_end;
static lv_obj_t *lable_voidce;

static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void set_zoom(void * img, int32_t v)
{
    lv_img_set_zoom(img, v);
}

static void music_event_cb(lv_event_t * e)
{
    static int i;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *  obj= lv_event_get_target(e);
    lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(e);
    if(code == LV_EVENT_PRESSED){
        if(obj != btn1){
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, img);
            lv_anim_set_exec_cb(&a, set_zoom);
            lv_anim_set_values(&a, 300, 255);
            lv_anim_set_time(&a, 500);
            lv_anim_start(&a);
        }else if(obj == btn1){
            if(i == 0){
                lv_img_set_src(img, &image16);
                i = 1;
            }else{
                lv_img_set_src(img, &image17);
                i = 0;
            }
        }
        if(obj == btn0){
            printf("back\n");
        }else if(obj == btn1){
            if(i == 0){
                printf("stop\n");
            }else{
                printf("run\n");
            }
        }else if(obj == btn2){
            printf("next\n");
        }
    }else if(code == LV_EVENT_VALUE_CHANGED){
        printf("%d\n", lv_slider_get_value(obj));
    }
}

static lv_obj_t* music_create(void)
{
    lv_obj_t *menu = obj_read(page_head, "menu");

    lv_obj_t *music = lv_obj_create(menu);
    lv_obj_set_size(music, 190, 210);
    lv_obj_set_style_pad_top(music, 0, 0);
    lv_obj_set_style_pad_bottom(music, 0, 0);
    lv_obj_set_style_pad_left(music, 0, 0);
    lv_obj_set_style_pad_right(music, 0, 0);
    lv_obj_align(music, LV_ALIGN_LEFT_MID, 0, 15);
    
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, music);
    lv_anim_set_values(&a, 240, 50);
    lv_anim_set_time(&a, 500);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, 0);
    lv_style_set_shadow_width(&style_btn, 0);
    lv_style_set_border_width(&style_btn, 0);


    lv_obj_t *control = lv_obj_create(music);
    lv_obj_set_size(control, 185, 120);
    lv_obj_align(control, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_style_init(&style_control);
    lv_style_set_border_width(&style_control, 0);
    lv_style_set_radius(&style_control, 10);
    lv_style_set_pad_all(&style_control, 0);
    lv_style_set_bg_color(&style_control, lv_color_hex(0xc0c0c0));
    lv_obj_add_style(control, &style_control, 0);

    btn0 = lv_btn_create(control);
    lv_obj_set_size(btn0, 50, 50);
    lv_obj_align(btn0, LV_ALIGN_TOP_MID, -60, 5);
    lv_obj_t *img0 = lv_img_create(btn0);
    lv_img_set_src(img0, &image15);
    lv_obj_center(img0);
    lv_obj_add_style(btn0, &style_btn, 0);
    lv_obj_add_event_cb(btn0, music_event_cb, LV_EVENT_PRESSED, (void *)img0);
    lv_obj_add_event_cb(btn0, music_event_cb, LV_EVENT_SHORT_CLICKED, (void *)img0);

    btn1 = lv_btn_create(control);
    lv_obj_set_size(btn1, 50, 50);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_t *img1 = lv_img_create(btn1);
    lv_img_set_src(img1, &image17);
    lv_obj_center(img1);
    lv_obj_add_style(btn1, &style_btn, 0);
    lv_obj_add_event_cb(btn1, music_event_cb, LV_EVENT_PRESSED, (void *)img1);
    lv_obj_add_event_cb(btn1, music_event_cb, LV_EVENT_SHORT_CLICKED, (void *)img1);

    btn2 = lv_btn_create(control);
    lv_obj_set_size(btn2, 50, 50);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, 60, 5);
    lv_obj_t *img2 = lv_img_create(btn2);
    lv_img_set_src(img2, &image18);
    lv_obj_center(img2);
    lv_obj_add_style(btn2, &style_btn, 0);
    lv_obj_add_event_cb(btn2, music_event_cb, LV_EVENT_PRESSED, (void *)img2);
    lv_obj_add_event_cb(btn2, music_event_cb, LV_EVENT_SHORT_CLICKED, (void *)img2);

    lv_style_init(&style_slider_main);
    lv_style_set_bg_opa(&style_slider_main, LV_OPA_50);
    lv_style_set_bg_color(&style_slider_main, lv_color_hex(0x7F7F7F));
    lv_style_set_radius(&style_slider_main, LV_RADIUS_CIRCLE);

    slider_time = lv_slider_create(control);
    lv_obj_set_size(slider_time, 165, 10);
    lv_obj_align_to(slider_time, btn1,LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(slider_time, music_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_style(slider_time, &style_slider_main, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider_time, lv_color_hex(0x000000), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_time, lv_color_hex(0x000000), LV_PART_KNOB);
    lv_obj_set_style_pad_top(slider_time, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(slider_time, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_right(slider_time, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_left(slider_time, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_top(slider_time, 5, LV_PART_KNOB|LV_STATE_PRESSED);
    lv_obj_set_style_pad_bottom(slider_time, 5, LV_PART_KNOB|LV_STATE_PRESSED);
    lv_obj_set_style_pad_right(slider_time, 5, LV_PART_KNOB|LV_STATE_PRESSED);
    lv_obj_set_style_pad_left(slider_time, 5, LV_PART_KNOB|LV_STATE_PRESSED);


    lv_slider_set_range(slider_time, 0 , 800);


    lable_start = lv_label_create(control);
    lv_label_set_text(lable_start, "00:00");
    lv_obj_set_style_text_font(lable_start, &lv_font_montserrat_20, 0);
    lv_obj_align_to(lable_start, slider_time, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lable_end = lv_label_create(control);
    lv_label_set_text(lable_end, "08:88");
    lv_obj_set_style_text_font(lable_end, &lv_font_montserrat_20, 0);
    lv_obj_align_to(lable_end, slider_time, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);

    lable_voidce = lv_label_create(control);
    lv_label_set_text(lable_voidce, "100%");
    lv_obj_align_to(lable_voidce, lable_start, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_font(lable_voidce, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lable_voidce, lv_color_hex(0x000000), 0);

    slider_voice = lv_slider_create(control);
    lv_obj_set_size(slider_voice, 105, 10);
    lv_obj_align(slider_voice,LV_ALIGN_BOTTOM_MID, 30, -8);
    lv_obj_add_style(slider_voice, &style_slider_main, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider_voice, lv_color_hex(0x000000), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_voice, lv_color_hex(0x000000), LV_PART_KNOB);
    lv_obj_set_style_pad_top(slider_voice, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(slider_voice, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_right(slider_voice, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_left(slider_voice, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_top(slider_voice, 5, LV_PART_KNOB|LV_STATE_PRESSED);
    lv_obj_set_style_pad_bottom(slider_voice, 5, LV_PART_KNOB|LV_STATE_PRESSED);
    lv_obj_set_style_pad_right(slider_voice, 5, LV_PART_KNOB|LV_STATE_PRESSED);
    lv_obj_set_style_pad_left(slider_voice, 5, LV_PART_KNOB|LV_STATE_PRESSED);

    return music;
}

static void music_delete(lv_obj_t* music)
{
    lv_obj_del(music);
}

void music_page_add(struct page_list *head)
{
    page_add(head, "music", music_create, music_delete);
}

void music_cmd_write(struct cmd_list *head, struct cmd_data cmd)
{
    cmd_write(head, "music", cmd);
}

void music_cmd_read(struct cmd_list *head, struct cmd_data *cmd)
{
    cmd_read(cmd_head, "music", cmd, 1);
}

void music_cmd_add(struct cmd_list *head)
{
    cmd_add(head, "music");
}

void music_cmd_handle(void)
{
    struct cmd_data cmd;
    music_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"music create") == 0){
        page_delete(page_head, cmd.cmd_info.info);
        page_create(page_head, "music");
    }
}

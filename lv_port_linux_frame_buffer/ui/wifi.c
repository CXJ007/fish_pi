#include "ui.h"

#include "includes.h"
#include "common/wpa_ctrl.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "utils/edit.h"
#include "utils/list.h"
#include "common/version.h"
#include "common/ieee802_11_defs.h"

static lv_style_t style_btn;
static lv_style_t style_cont;
static lv_obj_t *wifi;
static lv_obj_t *kb;
static lv_obj_t *ta;

static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void kb_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = lv_event_get_target(e);
    lv_obj_t * ta = lv_event_get_user_data(e);
    if(code == LV_EVENT_READY){
        printf("%s\n",lv_textarea_get_text(ta));
    }else if(code == LV_EVENT_CANCEL){
        printf("LV_EVENT_CANCEL\n");
        lv_obj_clear_flag(wifi, LV_OBJ_FLAG_HIDDEN);//对象隐藏
        lv_obj_del(kb);
        lv_obj_del(ta);
    }
    
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * lable = lv_obj_get_child(btn, 0);
    if(code == LV_EVENT_CLICKED){
        printf("%s\n",lv_label_get_text(lable));
        lv_obj_add_flag(wifi, LV_OBJ_FLAG_HIDDEN);//对象隐藏
        kb = lv_keyboard_create(lv_scr_act());
        lv_obj_align(kb, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_set_size(kb, 190, 100);
        
        ta = lv_textarea_create(lv_scr_act());
        lv_obj_align(ta, LV_ALIGN_TOP_RIGHT, 0, 30);
        lv_textarea_set_placeholder_text(ta, "PASS:");
        lv_obj_set_size(ta, 190, 80);
        lv_keyboard_set_textarea(kb, ta);

        lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_READY, ta);
        lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_CANCEL, ta);
    }
}

static void add_list_btn(lv_obj_t * parent)
{
    int i = 0;
    while(i<15){
        i++;
        lv_obj_t *btn= lv_obj_create(parent);
        lv_obj_add_style(btn, &style_btn, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xc0c0c0), LV_STATE_FOCUSED);
        lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_t * name_label = lv_label_create(btn);
        lv_label_set_text(name_label, "testlfjs");
        lv_obj_set_style_text_font(name_label, &myFont, 0);
        lv_obj_align(name_label, LV_ALIGN_LEFT_MID, 0, 0);
    }
}

static lv_obj_t* wifi_create(void)
{
    lv_obj_t *menu = obj_read(page_head, "menu");

    lv_style_init(&style_cont);
    lv_style_set_bg_color(&style_cont, lv_color_hex(0xFFFFFF));
    lv_style_set_border_width(&style_cont, 0);
    lv_style_set_shadow_width(&style_cont, 0);
    lv_style_set_radius(&style_cont, 0);
    lv_style_set_pad_all(&style_cont, 0);
    lv_style_set_pad_row(&style_cont, 0);

    wifi = lv_obj_create(menu);
    lv_obj_set_size(wifi, 190, 210);
    lv_obj_align(wifi, LV_ALIGN_LEFT_MID, 0, 15);
    lv_obj_set_flex_flow(wifi, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(wifi, &style_cont, 0);
    

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, wifi);
    lv_anim_set_values(&a, 240, 50);
    lv_anim_set_time(&a, 500);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    lv_style_init(&style_btn);
    lv_style_set_height(&style_btn,40);
    lv_style_set_width(&style_btn,185);
    lv_style_set_border_width(&style_btn, 1);
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_pad_all(&style_btn, 0);

    add_list_btn(wifi);
    struct wpa_ctrl *ctr = NULL;
    ctr = wpa_ctrl_open("/var/run/wpa_supplicant/wlan0");
    return wifi;
}

static void wifi_delete(lv_obj_t* obj)
{
    if(lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)){//检测隐藏判断键盘还在不
        lv_obj_del(kb);
        lv_obj_del(ta);
    }
    lv_obj_del(obj);
    lv_style_reset(&style_btn);
    lv_style_reset(&style_cont);
}

void wifi_page_add(struct page_list *head)
{
    page_add(head, "wifi", wifi_create, wifi_delete);
}

void wifi_cmd_write(struct cmd_list *head, struct cmd_data cmd)
{
    cmd_write(head, "wifi", cmd);
}

void wifi_cmd_read(struct cmd_list *head, struct cmd_data *cmd)
{
    cmd_read(cmd_head, "wifi", cmd, 1);
}

void wifi_cmd_add(struct cmd_list *head)
{
    cmd_add(head, "wifi");
}

void wifi_cmd_handle(void)
{
    struct cmd_data cmd;
    wifi_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"wifi create") == 0){
        page_delete(page_head, cmd.cmd_info.info);
        page_create(page_head, "wifi"); 
    }
}
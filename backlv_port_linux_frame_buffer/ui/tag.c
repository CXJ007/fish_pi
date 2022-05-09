#include "ui.h"

LV_IMG_DECLARE(image12);

static lv_obj_t* tag_create(void)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_t *menu = obj_read(page_head, "menu");
    lv_obj_t *tag = lv_obj_create(menu);
    lv_obj_set_size(tag, 240, 30);
    lv_obj_align(tag, LV_ALIGN_TOP_MID, 0, 0);

    static lv_style_t style_tag;
    lv_style_init(&style_tag);
    lv_style_set_border_width(&style_tag, 0);
    lv_style_set_radius(&style_tag, 0);
    lv_style_set_pad_all(&style_tag, 0);
    lv_style_set_bg_color(&style_tag, lv_color_hex(0xc0c0c0));
    lv_obj_add_style(tag, &style_tag, 0);
    

    lv_obj_t * clock = lv_label_create(tag);
    lv_label_set_text(clock,"12:13:12");
    lv_obj_align(clock, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_text_font(clock, &lv_font_montserrat_20, 0);

    lv_obj_t *img11 = lv_img_create(tag);
    lv_img_set_src(img11, &image12);
    lv_obj_align(img11, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * fan = lv_label_create(tag);
    lv_label_set_text(fan,":956848");
    lv_obj_align(fan,LV_ALIGN_LEFT_MID, 30, 0);
    lv_obj_set_style_text_font(fan, &lv_font_montserrat_20, 0);

    pthread_mutex_unlock(&lvgl_mutex);

    return tag;
}

static void tag_delete(lv_obj_t* tag)
{
    pthread_mutex_lock(&lvgl_mutex);
    lv_obj_del(tag);
    pthread_mutex_unlock(&lvgl_mutex);
}

void tag_page_add(struct page_list *head)
{
    page_add(head, "tag", tag_create, tag_delete);
}
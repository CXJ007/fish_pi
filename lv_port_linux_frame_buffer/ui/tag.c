#include "ui.h"

LV_IMG_DECLARE(image12);

static lv_obj_t *tag_clock;
static lv_obj_t *fan;
static lv_timer_t * tag_time;
static lv_style_t style_tag;
static lv_style_t style_btn;

static char clockbuf[50];
static char fanbuf[50];
static int h,m,s;
static char datebuf[20];


//curl https://api.bilibili.com/x/relation/stat?vmid=224654171
static void tag_timer(lv_timer_t * timer)
{  
    s++;
    if(s >= 60){
        m++;
        s = 0;
    }
    if(m >= 60){
        h++;
        m = 0;
    }
    if(h >= 24){
        h = 0;
    }
    sprintf(clockbuf, "%d%d:%d%d:%d%d", h/10, h%10, m/10, m%10, s/10, s%10);
    sprintf(sys_date, "%s:%d:%d:%d:", datebuf ,h ,m ,s);
    //printf("%s\n", sys_date);
    lv_label_set_text(tag_clock, clockbuf);
}



static void fan_sync(void)
{
    FILE * fp;
    char buf[50];
    int len;
    memset(buf,'\0', sizeof(buf));
    memset(fanbuf,'\0', sizeof(fanbuf));
    fp = popen("curl -s https://api.bilibili.com/x/relation/stat?vmid=224654171 | jq .data.follower", "r");
    fread(buf, sizeof(char),sizeof(buf) ,fp);
    pclose(fp);
    len = strlen(buf)-1;
    if(len > 0){
        strncpy(fanbuf, buf, len);
    }else{
        strcpy(fanbuf, "no net");
    }
    lv_label_set_text(fan, fanbuf);
}

static void tag_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED){
        fan_sync();
    }
}

static lv_obj_t* tag_create(void)
{
    char *delim = ":";
    char buf[50];
    strcpy(buf, sys_date);
    strcpy(datebuf, strtok(buf, delim));
    h = atoi(strtok(NULL, delim));
    m = atoi(strtok(NULL, delim));
    s = atoi(strtok(NULL, delim));

    lv_obj_t *menu = obj_read(page_head, "menu");
    lv_obj_t *tag = lv_obj_create(menu);
    lv_obj_set_size(tag, 240, 30);
    lv_obj_align(tag, LV_ALIGN_TOP_MID, 0, 0);

    lv_style_init(&style_tag);
    lv_style_set_border_width(&style_tag, 0);
    lv_style_set_radius(&style_tag, 0);
    lv_style_set_pad_all(&style_tag, 0);
    lv_style_set_bg_color(&style_tag, lv_color_hex(0xc0c0c0));
    lv_obj_add_style(tag, &style_tag, 0);
    
    sprintf(clockbuf, "%d%d:%d%d:%d%d", h/10, h%10, m/10, m%10, s/10, s%10);
    tag_clock = lv_label_create(tag);
    lv_label_set_text(tag_clock, clockbuf);
    lv_obj_align(tag_clock, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_text_font(tag_clock, &lv_font_montserrat_20, 0);

    
    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, 0);
    lv_style_set_shadow_width(&style_btn, 0);
    lv_style_set_border_width(&style_btn, 0);

    lv_obj_t *btn0 = lv_btn_create(tag);
    lv_obj_set_size(btn0, 30, 30);
    lv_obj_t *img12 = lv_img_create(btn0);
    lv_img_set_src(img12, &image12);
    lv_obj_center(img12);
    lv_obj_align(btn0, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_style(btn0, &style_btn, 0);
    lv_obj_add_event_cb(btn0, tag_event_cb, LV_EVENT_CLICKED, NULL);
    
    fan = lv_label_create(tag);
    lv_label_set_text(fan,fanbuf);
    lv_obj_align(fan,LV_ALIGN_LEFT_MID, 30, 0);
    lv_obj_set_style_text_font(fan, &lv_font_montserrat_20, 0);

    tag_time = lv_timer_create(tag_timer, 1000,  NULL);

    return tag;
}

static void tag_delete(lv_obj_t* tag)
{
    sprintf(sys_date, "%s:%d:%d:%d:", datebuf ,h ,m ,s);
    lv_timer_del(tag_time);
    lv_obj_del(tag);
    lv_style_reset(&style_tag);
    lv_style_reset(&style_btn);
}

void tag_page_add(struct page_list *head)
{
    page_add(head, "tag", tag_create, tag_delete);
}

void tag_cmd_write(struct cmd_list *head, struct cmd_data cmd)
{
    cmd_write(head, "tag", cmd);
}

void tag_cmd_read(struct cmd_list *head, struct cmd_data *cmd)
{
    cmd_read(cmd_head, "tag", cmd, 1);
}

void tag_cmd_add(struct cmd_list *head)
{
    cmd_add(head, "tag");
}

void tag_cmd_handle(void)
{
    struct cmd_data cmd;
    tag_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"time sync") == 0){
        char *delim = ":";
        strcpy(datebuf, strtok( cmd.cmd_info.info, delim));
        h = atoi(strtok(NULL, delim));
        m = atoi(strtok(NULL, delim));
        s = atoi(strtok(NULL, delim));
        sprintf(clockbuf, "%d%d:%d%d:%d%d", h/10, h%10, m/10, m%10, s/10, s%10);
        lv_label_set_text(tag_clock, clockbuf);
    }
}

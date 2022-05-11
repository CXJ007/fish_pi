#include "ui.h"

LV_IMG_DECLARE(image13);
LV_IMG_DECLARE(image14);

static lv_obj_t * roller_h;
static lv_obj_t * roller_m;
static lv_obj_t * roller_s;
static lv_obj_t *btn0;
static lv_obj_t *btn1;

pthread_mutex_t time_mutex;
static lv_timer_t * lvgl_timer;
static int h, m, s;
static int seth, setm, sets;
static char datebuf[20];
static int sync_home, sync_tag, sync_fan;
static char fanbuf[50];

//curl https://api.bilibili.com/x/relation/stat?vmid=224654171

static void sys_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

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
                sprintf(cmd.cmd_info.info, "%s:%d:%d",datebuf, h, m);
                home_cmd_write(cmd_head, cmd);
            }
        }
        if(sync_fan == 1){
            if(page_check(page_head, "tag") == 1){
                sync_fan = 0;
                struct cmd_data cmd;
                strcpy(cmd.cmd_name.name, "fan sync");
                strcpy(cmd.cmd_info.info, fanbuf);
                tag_cmd_write(cmd_head, cmd);
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
}

void sys_time_init(void)
{
    FILE * fp;
    char buf[50];
    char *delim = ":";
    fp = popen("date +%Y-%m-%d:%T:", "r");
    fread(buf, sizeof(char),sizeof(buf) ,fp);
    pclose(fp);
    pthread_mutex_lock(&time_mutex);
    strcpy(datebuf, strtok(buf, delim));
    h = atoi(strtok(NULL, delim));
    m = atoi(strtok(NULL, delim));
    s = atoi(strtok(NULL, delim));
    sync_home = 1;
    sync_tag = 1;
    pthread_mutex_unlock(&time_mutex);

    lv_obj_add_event_cb(lv_scr_act(), sys_event_cb, MY_EVENT_TIME , NULL);
    lvgl_timer = lv_timer_create(sys_time_sync, 1000,  NULL);
}


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
    lv_obj_t *  obj= lv_event_get_target(e);
    lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        if(obj == roller_h){
            pthread_mutex_lock(&time_mutex);
            seth = atoi(buf);
            pthread_mutex_unlock(&time_mutex);
        }else if(obj == roller_m){
            pthread_mutex_lock(&time_mutex);
            setm = atoi(buf);
            pthread_mutex_unlock(&time_mutex);
        }else if(obj == roller_s){
            pthread_mutex_lock(&time_mutex);
            sets = atoi(buf);
            pthread_mutex_unlock(&time_mutex);
        }
        //LV_LOG_USER("Selected value: %s", buf);
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
            struct cmd_data cmd;
            strcpy(cmd.cmd_name.name, "time sync");
            time_cmd_write(cmd_head, cmd);
        }else if(obj == btn1){
            struct cmd_data cmd;
            strcpy(cmd.cmd_name.name, "time set");
            time_cmd_write(cmd_head, cmd);
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

    roller_h = lv_roller_create(time);
    lv_roller_set_options(roller_h, opts_h, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_h, 3);
    lv_obj_set_width(roller_h, 50);
    lv_obj_set_style_text_font(roller_h, &lv_font_montserrat_16, 0);
    lv_obj_add_style(roller_h, &style_roller, LV_PART_SELECTED);
    lv_obj_align(roller_h, LV_ALIGN_CENTER, -55, -10);
    lv_obj_add_event_cb(roller_h, time_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    pthread_mutex_lock(&time_mutex);
    lv_roller_set_selected(roller_h, h, LV_ANIM_OFF);
    pthread_mutex_unlock(&time_mutex);

    roller_m = lv_roller_create(time);
    lv_roller_set_options(roller_m, opts_ms, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_m, 3);
    lv_obj_set_width(roller_m, 50);
    lv_obj_set_style_text_font(roller_m, &lv_font_montserrat_16, 0);
    lv_obj_add_style(roller_m, &style_roller, LV_PART_SELECTED);
    lv_obj_align(roller_m, LV_ALIGN_CENTER, 0, -10);
    lv_obj_add_event_cb(roller_m, time_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    pthread_mutex_lock(&time_mutex);
    lv_roller_set_selected(roller_m, m, LV_ANIM_OFF);
    pthread_mutex_unlock(&time_mutex);

    roller_s = lv_roller_create(time);
    lv_roller_set_options(roller_s, opts_ms, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_s, 3);
    lv_obj_set_width(roller_s, 50);
    lv_obj_set_style_text_font(roller_s, &lv_font_montserrat_16, 0);
    lv_obj_add_style(roller_s, &style_roller, LV_PART_SELECTED);
    lv_obj_align(roller_s, LV_ALIGN_CENTER, 55, -10);
    lv_obj_add_event_cb(roller_s, time_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    pthread_mutex_lock(&time_mutex);
    lv_roller_set_selected(roller_s, s, LV_ANIM_OFF);
    pthread_mutex_unlock(&time_mutex);

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

    pthread_mutex_lock(&time_mutex);
    seth = h;
    setm = m;
    sets = s;
    pthread_mutex_unlock(&time_mutex);
    
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

static void time_data_sync(void)
{
    //时间没有mutex
    FILE * fp;
    char buf[50];
    memset(buf,'\0', sizeof(buf));
    char *delim = ":";
    fp = popen("date +%Y-%m-%d:%T:", "r");
    fread(buf, sizeof(char),sizeof(buf) ,fp);
    pclose(fp);
    pthread_mutex_lock(&time_mutex);
    strcpy(datebuf, strtok(buf, delim));
    seth = atoi(strtok(NULL, delim));
    setm = atoi(strtok(NULL, delim));
    sets = atoi(strtok(NULL, delim));
    h = seth;
    m = setm;
    s = sets;
    pthread_mutex_unlock(&time_mutex);
    pthread_mutex_lock(&lvgl_mutex);
    //pthread_mutex_lock(&time_mutex);
    lv_roller_set_selected(roller_h, h, LV_ANIM_OFF);
    lv_roller_set_selected(roller_m, m, LV_ANIM_OFF);
    lv_roller_set_selected(roller_s, s, LV_ANIM_OFF);
    //pthread_mutex_unlock(&time_mutex);
    pthread_mutex_unlock(&lvgl_mutex);
    sync_home = 1;
    sync_tag = 1;
}

void fan_sync(void)
{
    FILE * fp;
    char buf[50];
    int len;
    memset(buf,'\0', sizeof(buf));
    memset(fanbuf,'\0', sizeof(fanbuf));
    char *delim = ":";
    fp = popen("curl -s https://api.bilibili.com/x/relation/stat?vmid=224654171 | jq .data.follower", "r");
    fread(buf, sizeof(char),sizeof(buf) ,fp);
    pclose(fp);
    len = strlen(buf)-1;
    if(len > 0){
        strncpy(fanbuf, buf, len);
    }else{
        strcpy(fanbuf, "no net");
    }
    sync_fan = 1;
}

void time_cmd_handle(void)
{
    struct cmd_data cmd;
    time_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"time create") == 0){
        page_create(page_head, "time"); 
    }else if(strcmp(cmd.cmd_name.name,"time sync") == 0){
        time_data_sync();
        fan_sync();
    }else if(strcmp(cmd.cmd_name.name,"time set") == 0){
        pthread_mutex_lock(&time_mutex);
        h = seth;
        m = setm;
        s = sets;
        sync_home = 1;
        sync_tag = 1;
        pthread_mutex_unlock(&time_mutex);
        printf("%d:%d:%d\n", seth,setm,sets);
    }
}
#include "ui.h"

#include "includes.h"
#include "common/wpa_ctrl.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "utils/edit.h"
#include "utils/list.h"
#include "common/version.h"
#include "common/ieee802_11_defs.h"

LV_FONT_DECLARE(myFont1);
#define MY_SYMBOL_WIFI "\xEE\x9A\xB5"
#define MY_SYMBOL_LOCK "\xEE\x98\x80"
#define MY_SYMBOL_QUES "\xEE\x99\x85"

#define WPA_PATH "/var/run/wpa_supplicant/wlan0"
#define WIFI_LEN 20

static struct wifi_mesg{
    char mesg[100];
    char bssid[20];
    char ssid[20];
    int id;    //wpa里面的id
    int statu; //0连接 1知道密码未连接 2未知wifi 3wifi不存在
}wifi_arr[WIFI_LEN];

static char ip_address[20];

static lv_style_t style_btn;
static lv_style_t style_cont;
static lv_obj_t *wifi;
static lv_obj_t *kb;
static lv_obj_t *ta;
static lv_obj_t *lable_wifi;

static pthread_t  wifi_id;
static pthread_mutex_t wifi_mutex;
static pthread_cond_t  wifi_cond;
static int ready = 0;                    //线程唤醒丢失
static char cmdbuf[50];              //传递wpa命令
        

static void *wpa_handle(void *argc);
static void send_cmd(char *buf);
static int get_wifi_statu(char *name);

static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void kb_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * keyb = lv_event_get_target(e);
    lv_obj_t * texta = lv_event_get_user_data(e);
    if(code == LV_EVENT_READY){
        printf("%s\n",lv_textarea_get_text(texta));
    }else if(code == LV_EVENT_CANCEL){
        lv_obj_clear_flag(wifi, LV_OBJ_FLAG_HIDDEN);//对象隐藏
        lv_obj_del(keyb);
        lv_obj_del(texta);
        lv_obj_del(lable_wifi);
    }
    
}

static void make_kb(lv_obj_t *lable)
{
    lv_obj_add_flag(wifi, LV_OBJ_FLAG_HIDDEN);//对象隐藏
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_align(kb, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_size(kb, 190, 100);
    
    ta = lv_textarea_create(lv_scr_act());
    lv_obj_align(ta, LV_ALIGN_TOP_RIGHT, 0, 80);
    lv_textarea_set_placeholder_text(ta, "PASS:");
    lv_obj_set_size(ta, 190, 40);
    lv_keyboard_set_textarea(kb, ta);

    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_READY, ta);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_CANCEL, ta);

    lable_wifi = lv_label_create(lv_scr_act());
    lv_label_set_text(lable_wifi, lv_label_get_text(lable));
    lv_obj_align(lable_wifi,LV_ALIGN_TOP_RIGHT, 0, 35);
    lv_obj_set_size(lable_wifi, 190, 40);
    lv_obj_set_style_text_font(lable_wifi, &myFont, 0);
    lv_obj_set_style_text_align(lable_wifi, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_border_color(lable_wifi, lv_color_hex(0xe6e2e6), 0);
    lv_obj_set_style_radius(lable_wifi, 10, 0);
    lv_obj_set_style_border_width(lable_wifi, 2, 0);

}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED){
        lv_obj_t * btn = lv_event_get_target(e);
        lv_obj_t * lable = lv_obj_get_child(btn, 0);
        printf("%s\n",lv_label_get_text(lable));
        if(get_wifi_statu(lv_label_get_text(lable)) == 0){
            lv_obj_t * mbox = lv_msgbox_create(NULL, lv_label_get_text(lable), ip_address, NULL, true);
            lv_obj_set_style_text_font(mbox, &lv_font_montserrat_20, 0);
            lv_obj_set_style_bg_color(lv_msgbox_get_close_btn(mbox), lv_color_hex(0xc0c0c0), 0);
            lv_obj_set_style_shadow_width(lv_msgbox_get_close_btn(mbox), 0, 0);
            lv_obj_set_size(mbox, 230, 100);
            lv_obj_center(mbox);
        }else if(get_wifi_statu(lv_label_get_text(lable)) == 1){
            static const char * btns[] ={LV_SYMBOL_KEYBOARD, LV_SYMBOL_OK, ""};
            lv_obj_t * mbox = lv_msgbox_create(NULL, lv_label_get_text(lable), "password or connection", btns, true);
            lv_obj_set_style_bg_color(lv_msgbox_get_close_btn(mbox), lv_color_hex(0xc0c0c0), 0);
            lv_obj_set_style_shadow_width(lv_msgbox_get_close_btn(mbox), 0, 0);
            lv_obj_set_size(mbox, 230, 120);
            lv_obj_center(mbox);
            lv_obj_add_event_cb(mbox, btn_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
        }else if(get_wifi_statu(lv_label_get_text(lable)) == 2){
            make_kb(lable);
        }
    } else if(code == LV_EVENT_VALUE_CHANGED){
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *obj = lv_event_get_current_target(e);
        lv_obj_t *mbox = lv_obj_get_parent(btn);
        if(lv_msgbox_get_active_btn_text(obj) == LV_SYMBOL_KEYBOARD){
            printf("kb\n");
            make_kb(lv_msgbox_get_title(mbox));
        }else if(lv_msgbox_get_active_btn_text(obj) == LV_SYMBOL_OK){
            printf("ok\n");
        }
        lv_msgbox_close(mbox);
    }
}

//层层过滤确定wifi状态
static void add_list_btn(lv_obj_t * parent)
{
    int i = 0;
    while(1){
        if(i > WIFI_LEN) break;
        if(wifi_arr[i].statu == 3) break;

        lv_obj_t *btn= lv_obj_create(parent);
        lv_obj_add_style(btn, &style_btn, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xc0c0c0), LV_STATE_FOCUSED);
        lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
       
        lv_obj_t * name_label = lv_label_create(btn);
        lv_label_set_text(name_label, wifi_arr[i].ssid);
        lv_obj_set_style_text_font(name_label, &myFont, 0);
        lv_obj_align(name_label, LV_ALIGN_LEFT_MID, 5, 0);

        lv_obj_t * statu_label = lv_label_create(btn);
        if(wifi_arr[i].statu == 0) lv_label_set_text(statu_label, MY_SYMBOL_WIFI);
        else if(wifi_arr[i].statu == 1) lv_label_set_text(statu_label, MY_SYMBOL_LOCK);
        else if(wifi_arr[i].statu == 2) lv_label_set_text(statu_label, MY_SYMBOL_QUES);
        lv_obj_set_style_text_font(statu_label, &myFont1, 0);
        lv_obj_align(statu_label, LV_ALIGN_RIGHT_MID, -10, 0);
        i++;
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
    lv_style_set_border_color(&style_btn, lv_color_hex(0xc0c0c0));
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_pad_all(&style_btn, 0);

    pthread_mutex_init(&wifi_mutex, NULL);
	pthread_cond_init(&wifi_cond, NULL);
    pthread_create(&wifi_id, NULL, wpa_handle, NULL);
    pthread_detach(wifi_id);

    return wifi;
}

static void wifi_delete(lv_obj_t* obj)
{
    if(lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)){//检测隐藏判断键盘还在不
        lv_obj_del(kb);
        lv_obj_del(ta);
        lv_obj_del(lable_wifi);
    }

    lv_obj_del(obj);
    lv_style_reset(&style_btn);
    lv_style_reset(&style_cont);
    send_cmd("QUIT");
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

static void wifi_statu_sync(void)
{
    send_cmd("SCAN");
    send_cmd("SCAN_RESULTS");
    send_cmd("LIST_NETWORKS");
    send_cmd("STATUS");
}

static int get_wifi_statu(char *name)
{
    int num;
    for(num=0;num<WIFI_LEN;num++){
        if(strcmp(wifi_arr[num].ssid, name) == 0)
            return wifi_arr[num].statu;
    }
    return 3;
}

//清空不存在wifi
static void wifi_free(int num)
{
    for(;num<WIFI_LEN;num++){
        memset(wifi_arr[num].mesg, '\0', sizeof(wifi_arr[num].mesg));
        memset(wifi_arr[num].bssid, '\0', sizeof(wifi_arr[num].bssid));
        memset(wifi_arr[num].ssid, '\0', sizeof(wifi_arr[num].ssid));
    }
}

// bssid=92:78:41:dc:6a:a4
// freq=2412
// ssid=pi
// id=0
// mode=station
// wifi_generation=4
// pairwise_cipher=CCMP
// group_cipher=CCMP
// key_mgmt=WPA2-PSK
// wpa_state=COMPLETED
// ip_address=192.168.137.60
// p2p_device_address=c0:21:0d:72:f4:ac
// address=c0:21:0d:72:f4:ac
// uuid=3b28b424-d20f-5d84-a0c4-5ebfefebbb55
//还是popen方便
static void wpa_status(char *retbuf)
{
    FILE * fb;
    FILE * fp;
    int i = 0;
    char buf[30];

    memset(buf, '\0', sizeof(buf));
    fb = fopen("/tmp/wifi_tmp", "w+");
    fwrite(retbuf,1,256,fb);
    fclose(fb);
    fp = popen("cat /tmp/wifi_tmp | grep bssid | sed -e 's/bssid=//'", "r");
    fread(buf, sizeof(char),sizeof(buf) ,fp);
    pclose(fp);
    while(1){
        if(i>WIFI_LEN) break;
        if(strstr(buf,wifi_arr[i].bssid) != NULL){ //多出\n
            wifi_arr[i].statu = 0;
            break;
        }
        i++;
    }
    memset(buf, '\0', sizeof(buf));
    fp = popen("cat /tmp/wifi_tmp | grep ip_address | sed -e 's/ip_address=//'", "r");
    fread(buf, sizeof(char),sizeof(buf) ,fp);
    pclose(fp);
    strcpy(ip_address, buf);
}



// network id / ssid / bssid / flags
// 0	pi	any	[CURRENT]
// 1	fishpi	any	[DISABLED]
//修改wifi_arr id是否已经wifi有密码
static void wpa_list(char *retbuf)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int id;
    char *p;
    char *delim0 = "\n";
    char *delim1 = "\t";
    p = strtok(retbuf, delim0);
    p = strtok(NULL, delim0);
    while(p != NULL){
        strcpy(wifi_arr[i].mesg, p);
        i++;
        if(i > WIFI_LEN)break; //超出WIFI_LEN
        p = strtok(NULL, delim0);
    }
    for(j=0;j<i;j++){
        k = 0;
        p = strtok(wifi_arr[j].mesg, delim1);//id
        p = strtok(NULL, delim1);//ssid
        while(1){ //在scan出来的wifi中遍历
            if(k >= WIFI_LEN)break;
            if(wifi_arr[k].statu == 3)break;
            if(strcmp(wifi_arr[k].ssid, p) == 0){
                wifi_arr[k].id = j;
                wifi_arr[k].statu = 1;
                break;
            }
            k++;
        }
    }
    // printf("%s  %s  %d  %d\n", wifi_arr[0].bssid, wifi_arr[0].ssid, wifi_arr[0].statu, wifi_arr[0].id);
    // printf("%s  %s  %d  %d\n", wifi_arr[1].bssid, wifi_arr[1].ssid, wifi_arr[1].statu, wifi_arr[1].id);
    // printf("%s  %s  %d  %d\n", wifi_arr[2].bssid, wifi_arr[2].ssid, wifi_arr[2].statu, wifi_arr[2].id);
    // printf("%s  %s  %d  %d\n", wifi_arr[3].bssid, wifi_arr[3].ssid, wifi_arr[3].statu, wifi_arr[3].id);
    // printf("%s  %s  %d  %d\n", wifi_arr[4].bssid, wifi_arr[4].ssid, wifi_arr[4].statu, wifi_arr[4].id);
}

// bssid / frequency / signal level / flags / ssid
// 92:78:41:dc:6a:a4	2452	-60	[WPA2-PSK-CCMP][WPS][ESS]	pi
// 0a:74:9c:fb:16:d7	2452	-71	[ESS]	SUSE_Hostel
static void wpa_scan(char *retbuf)
{
    int i = 0;
    int j = 0;
    int k = 0;
    char *p;
    char *delim0 = "\n";
    char *delim1 = "\t";
    p = strtok(retbuf, delim0);
    p = strtok(NULL, delim0);
    if(p == NULL){
        wifi_arr[i].statu = 3;
    }
    while(p != NULL){
        strcpy(wifi_arr[i].mesg, p);
        wifi_arr[i].statu = 2;
        i++;
        if(i > WIFI_LEN)break; //超出WIFI_LEN 
        wifi_arr[i].statu = 3;//末尾wifi不存在
        p = strtok(NULL, delim0);
    }
    for(j=0;j<i;j++){
        k = 0;
        p = strtok(wifi_arr[j].mesg, delim1);
        while(p != NULL){
            if(k == 0) strcpy(wifi_arr[j].bssid, p);
            else if(k == 4) strcpy(wifi_arr[j].ssid, p);
            p = strtok(NULL, delim1);
            k++;
        }
    }
    wifi_free(i);
    // printf("11%s  %s  %d  %d\n", wifi_arr[0].bssid, wifi_arr[0].ssid, wifi_arr[0].statu, wifi_arr[0].id);
    // printf("11%s  %s  %d  %d\n", wifi_arr[1].bssid, wifi_arr[1].ssid, wifi_arr[1].statu, wifi_arr[1].id);
    // printf("11%s  %s  %d  %d\n", wifi_arr[2].bssid, wifi_arr[2].ssid, wifi_arr[2].statu, wifi_arr[2].id);
    // printf("11%s  %s  %d  %d\n", wifi_arr[3].bssid, wifi_arr[3].ssid, wifi_arr[3].statu, wifi_arr[3].id);
    // printf("11%s  %s  %d  %d\n", wifi_arr[4].bssid, wifi_arr[4].ssid, wifi_arr[4].statu, wifi_arr[4].id);
}

static void send_cmd(char *buf)
{
    pthread_mutex_lock(&wifi_mutex);
    ready = 1;
    memset(cmdbuf, '\0', sizeof(cmdbuf));
    strcpy(cmdbuf,buf);
    pthread_mutex_unlock(&wifi_mutex);
    pthread_cond_signal(&wifi_cond);//唤醒cmd线程

    pthread_mutex_lock(&wifi_mutex);
    pthread_cond_wait(&wifi_cond, &wifi_mutex);
    pthread_mutex_unlock(&wifi_mutex);
}


static void *wpa_handle(void *argc)
{
    char retbuf[256];
    int len;
    int ret;
    struct wpa_ctrl *ctr = NULL;

    ctr = wpa_ctrl_open(WPA_PATH);
    if(ctr == NULL){
        printf("wpa_err\n");
        pthread_exit((void *)0);
    }
    while(1){
        len = sizeof(retbuf);
        pthread_mutex_lock(&wifi_mutex);
		while(ready == 0){
			pthread_cond_wait(&wifi_cond, &wifi_mutex);
		}
        ready = 0;
        printf("wpa %s\n", cmdbuf);
        if(strcmp(cmdbuf, "QUIT") == 0){
            wpa_ctrl_close(ctr);
            pthread_mutex_unlock(&wifi_mutex);
            pthread_cond_signal(&wifi_cond);
            pthread_mutex_destroy(&wifi_mutex);
            pthread_cond_destroy(&wifi_cond);
            pthread_exit((void *)0);
        }
        ret = wpa_ctrl_request(ctr, cmdbuf, strlen(cmdbuf),retbuf,&len, NULL);
        if(ret == 0){
            if(strcmp(cmdbuf, "STATUS") == 0){
                //printf("%s\n%d\n",retbuf,len);
                wpa_status(retbuf);
            }else if(strcmp(cmdbuf, "LIST_NETWORKS") == 0){
                //printf("%s\n",retbuf);
                wpa_list(retbuf);
            }else if(strcmp(cmdbuf, "SCAN") == 0){
                //printf("%s\n",retbuf);
            }else if(strcmp(cmdbuf, "SCAN_RESULTS") == 0){
                //printf("%s\n",retbuf);
                wpa_scan(retbuf);              
            }
        }else {
            printf("wpa_err\n");
        }
        memset(retbuf, '\0', sizeof(retbuf));
        pthread_mutex_unlock(&wifi_mutex);
        pthread_cond_signal(&wifi_cond);
    }

}


void wifi_cmd_handle(void)
{
    struct cmd_data cmd;
    wifi_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"wifi create") == 0){
        pthread_mutex_lock(&lvgl_mutex);
        page_delete(page_head, cmd.cmd_info.info);
        page_create(page_head, "wifi"); 
        pthread_mutex_unlock(&lvgl_mutex);
        wifi_statu_sync();
        pthread_mutex_lock(&lvgl_mutex);
        add_list_btn(wifi);
        pthread_mutex_unlock(&lvgl_mutex);
    }
}
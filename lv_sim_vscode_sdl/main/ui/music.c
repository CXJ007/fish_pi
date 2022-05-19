#include "ui.h"

LV_IMG_DECLARE(image15);
LV_IMG_DECLARE(image16);
LV_IMG_DECLARE(image17);
LV_IMG_DECLARE(image18);
static lv_style_t style_btn;
static lv_style_t style_btn1;
static lv_style_t style_control;
static lv_style_t style_slider_main;
static lv_style_t style_cont;
static lv_obj_t *music;
static lv_obj_t *btn0;
static lv_obj_t *btn1;
static lv_obj_t *btn2;
static lv_obj_t *slider_time;
static lv_obj_t *slider_voice;
static lv_obj_t *lable_start;
static lv_obj_t *lable_end;
static lv_obj_t *lable_voidce;
static lv_obj_t *cont_col;
static lv_timer_t * music_slider_time;
static int voice = 100;
static int time_start = 0;
static int time_end = 0;
static pthread_t music_w_id;
static pthread_t music_r_id;
static int fd_fifo;					//创建有名管道，用于向mplayer发送命令
static int fd_pipe[2];				//创建无名管道,用于从mplayer读取命令
static pid_t mplayer_pid;           //mplayer的pid
static char cmd_buf[100];
static int pre_btn_id;
static int btn_music_id;
static int music_num;  
static pthread_mutex_t mplayer_mutex;
static pthread_cond_t  mplayer_cond;
static int start; 
static int ready;
static int btn_exchange;           

static void music_hand(void);
static void set_btn_state(lv_obj_t * parent, lv_obj_t * add_btn);

static void create_mplayer_timer(lv_timer_t * timer)
{   
    lv_event_send(music, MY_EVENT_TIME, NULL);
}

static void slider_set_time(lv_timer_t * timer)
{  
    char tmp[10]; 
    time_start++;
    if(time_start > time_end) time_start = 0;
    lv_slider_set_value(slider_time, time_start, LV_ANIM_ON);
    sprintf(tmp,"%d%d:%d%d", time_start/60/10, time_start/60%10, 
                            time_start%60/10, time_start%60%10);
    lv_label_set_text(lable_start, tmp);
    
}

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
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *  obj= lv_event_get_target(e);
    lv_obj_t *img = (lv_obj_t *)lv_event_get_user_data(e);
    struct cmd_data cmd;

    if(code == LV_EVENT_CLICKED){
        if(obj != btn1){
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, img);
            lv_anim_set_exec_cb(&a, set_zoom);
            lv_anim_set_values(&a, 300, 255);
            lv_anim_set_time(&a, 500);
            lv_anim_start(&a);
        }else if(obj == btn1){
            if(btn_exchange == 0){
                lv_img_set_src(img, &image16);
                btn_exchange = 1;
            }else{
                lv_img_set_src(img, &image17);
                btn_exchange = 0;
            }
        }
        if(obj == btn0){
            btn_music_id--;
            if(btn_music_id < 0) btn_music_id = music_num-1;
            set_btn_state(cont_col,lv_obj_get_child(cont_col,btn_music_id));
            lv_event_send(lv_obj_get_child(cont_col,btn_music_id), LV_EVENT_CLICKED, NULL);
        }else if(obj == btn1){
            if(btn_exchange == 0){
                strcpy(cmd.cmd_name.name, "music control");
                strcpy(cmd.cmd_info.info, "pause\n");
                music_cmd_write(cmd_head, cmd);
                lv_timer_pause(music_slider_time);
            }else{
                strcpy(cmd.cmd_name.name, "music control");
                strcpy(cmd.cmd_info.info, "pause\n");
                music_cmd_write(cmd_head, cmd);
                lv_timer_resume(music_slider_time);
            }
        }else if(obj == btn2){
            btn_music_id++;
            if(btn_music_id >= music_num) btn_music_id = 0;
            set_btn_state(cont_col,lv_obj_get_child(cont_col,btn_music_id));
            lv_event_send(lv_obj_get_child(cont_col,btn_music_id), LV_EVENT_CLICKED, NULL);
        }
    }else if(code == LV_EVENT_VALUE_CHANGED){
        if(obj == slider_time){
            char tmp[10];
            time_start = lv_slider_get_value(obj);
            sprintf(tmp,"%d%d:%d%d", time_start/60/10, time_start/60%10, 
                                    time_start%60/10, time_start%60%10);
            lv_label_set_text(lable_start, tmp);
            sprintf(tmp,"%d", time_start);
            strcpy(cmd.cmd_name.name, "music time");
            strcpy(cmd.cmd_info.info, tmp);
            music_cmd_write(cmd_head, cmd);
        }else if(obj == slider_voice){
            char tmp[10];
            voice = lv_slider_get_value(obj);
            sprintf(tmp,"%d%%", voice);
            lv_label_set_text(lable_voidce, tmp);
            sprintf(tmp,"%d", voice);
            strcpy(cmd.cmd_name.name, "music voice");
            strcpy(cmd.cmd_info.info, tmp);
            music_cmd_write(cmd_head, cmd);
        }
    }else if(code == MY_EVENT_TIME){
        music_hand();
    }
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * lable = lv_obj_get_child(btn, 0);
    set_btn_state(cont_col, btn);
    if(code == LV_EVENT_CLICKED){
        struct cmd_data cmd;
        strcpy(cmd.cmd_name.name, "music name");
        strcpy(cmd.cmd_info.info, lv_label_get_text(lable));
        music_cmd_write(cmd_head, cmd);
    }
}

static void set_btn_state(lv_obj_t * parent, lv_obj_t * add_btn)
{
    int id;
    id = lv_obj_get_child_id(add_btn);
    btn_music_id = id;
    lv_obj_t * clear_btn = lv_obj_get_child(parent, pre_btn_id);
    lv_obj_clear_state(clear_btn, LV_STATE_CHECKED);
    lv_obj_add_state(add_btn, LV_STATE_CHECKED);
    pre_btn_id = id;
    
}

static void add_list_btn(lv_obj_t * parent)
{
    char namebuf[256];
    FILE * fb;

    system("ls /home/cxj/Desktop/fish_pi/music | grep -E \"mp3|wav\" > /home/cxj/Desktop/fish_pi/music/music.lst");
    fb = fopen("/home/cxj/Desktop/fish_pi/music/music.lst", "r");
    memset(namebuf, '\0', sizeof(namebuf));
    fread(namebuf,sizeof(namebuf),1,fb);
    fclose(fb);
    char *delim = "\n";
    char *name = strtok(namebuf, delim);
    while(name != NULL) {
        music_num++;
        lv_obj_t *btn= lv_obj_create(parent);
        lv_obj_add_style(btn, &style_btn1, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xc0c0c0), LV_STATE_CHECKED);
        lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_t * name_label = lv_label_create(btn);
        lv_label_set_text(name_label, name);
        lv_obj_set_style_text_font(name_label, &myFont, 0);
        lv_obj_align(name_label, LV_ALIGN_LEFT_MID, 0, 0);
        name = strtok(NULL, delim);
    }
}

static lv_obj_t* music_create(void)
{    
    lv_timer_t * music_time;
    lv_obj_t *menu = obj_read(page_head, "menu");

    music = lv_obj_create(menu);
    lv_obj_set_size(music, 190, 210);
    lv_obj_set_style_pad_top(music, 0, 0);
    lv_obj_set_style_pad_bottom(music, 0, 0);
    lv_obj_set_style_pad_left(music, 0, 0);
    lv_obj_set_style_pad_right(music, 0, 0);
    lv_obj_align(music, LV_ALIGN_LEFT_MID, 0, 15);
    lv_obj_add_event_cb(music, music_event_cb, MY_EVENT_TIME , NULL);
    
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, music);
    lv_anim_set_values(&a, 240, 50);
    lv_anim_set_time(&a, 500);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);

    lv_style_init(&style_cont);
    lv_style_set_bg_color(&style_cont, lv_color_hex(0xFFFFFF));
    lv_style_set_border_width(&style_cont, 0);
    lv_style_set_shadow_width(&style_cont, 0);
    lv_style_set_radius(&style_cont, 0);
    lv_style_set_pad_all(&style_cont, 0);
    lv_style_set_pad_row(&style_cont, 0);

    cont_col = lv_obj_create(music);
    lv_obj_set_size(cont_col, 185, 85);
    lv_obj_align(cont_col, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(cont_col, &style_cont, 0);

    lv_style_init(&style_btn1);
    lv_style_set_height(&style_btn1,40);
    lv_style_set_width(&style_btn1,185);
    lv_style_set_border_width(&style_btn1, 1);
    lv_style_set_radius(&style_btn1, 10);
    lv_style_set_pad_all(&style_btn1, 0);
    
    add_list_btn(cont_col);
    set_btn_state(cont_col,lv_obj_get_child(cont_col,btn_music_id));

    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, 0);
    lv_style_set_shadow_width(&style_btn, 0);
    lv_style_set_border_width(&style_btn, 0);


    lv_obj_t *control = lv_obj_create(music);
    lv_obj_set_size(control, 185, 120);
    lv_obj_align(control, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_style_init(&style_control);
    lv_style_set_border_width(&style_control, 0);
    lv_style_set_radius(&style_control, 5);
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
    lv_obj_add_event_cb(btn0, music_event_cb, LV_EVENT_CLICKED, (void *)img0);

    btn1 = lv_btn_create(control);
    lv_obj_set_size(btn1, 50, 50);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_t *img1 = lv_img_create(btn1);
    lv_img_set_src(img1, &image17);
    lv_obj_center(img1);
    lv_obj_add_style(btn1, &style_btn, 0);
    lv_obj_add_event_cb(btn1, music_event_cb, LV_EVENT_CLICKED, (void *)img1);

    btn2 = lv_btn_create(control);
    lv_obj_set_size(btn2, 50, 50);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, 60, 5);
    lv_obj_t *img2 = lv_img_create(btn2);
    lv_img_set_src(img2, &image18);
    lv_obj_center(img2);
    lv_obj_add_style(btn2, &style_btn, 0);
    lv_obj_add_event_cb(btn2, music_event_cb, LV_EVENT_CLICKED, (void *)img2);

    lv_style_init(&style_slider_main);
    lv_style_set_bg_opa(&style_slider_main, LV_OPA_50);
    lv_style_set_bg_color(&style_slider_main, lv_color_hex(0x7F7F7F));
    lv_style_set_radius(&style_slider_main, LV_RADIUS_CIRCLE);

    slider_time = lv_slider_create(control);
    lv_obj_set_size(slider_time, 165, 10);
    lv_obj_align_to(slider_time, btn1,LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
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
    lv_obj_add_event_cb(slider_time, music_event_cb, LV_EVENT_VALUE_CHANGED, NULL);


    char tmp[10];
    lable_start = lv_label_create(control);
    sprintf(tmp,"%d%d:%d%d", time_start/60/10, time_start/60%10, 
                                    time_start%60/10, time_start%60%10);
    lv_label_set_text(lable_start, tmp);
    lv_obj_set_style_text_font(lable_start, &lv_font_montserrat_20, 0);
    lv_obj_align_to(lable_start, slider_time, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lable_end = lv_label_create(control);
    sprintf(tmp,"%d%d:%d%d", time_end/60/10, time_end/60%10, 
                                    time_end%60/10, time_end%60%10);
    lv_label_set_text(lable_end, tmp);
    lv_obj_set_style_text_font(lable_end, &lv_font_montserrat_20, 0);
    lv_obj_align_to(lable_end, slider_time, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);

    sprintf(tmp,"%d%%", voice);
    lable_voidce = lv_label_create(control);
    lv_label_set_text(lable_voidce, tmp);
    lv_obj_align_to(lable_voidce, lable_start, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_text_font(lable_voidce, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lable_voidce, lv_color_hex(0x000000), 0);

    slider_voice = lv_slider_create(control);
    lv_obj_set_size(slider_voice, 105, 10);
    lv_slider_set_range(slider_voice, 0 , 100);
    lv_slider_set_value(slider_voice, voice, LV_ANIM_OFF);
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
    lv_obj_add_event_cb(slider_voice, music_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    music_time = lv_timer_create(create_mplayer_timer, 1000,  NULL);
    lv_timer_set_repeat_count(music_time, 1);
    music_slider_time = lv_timer_create(slider_set_time, 1000,  NULL);
    lv_timer_pause(music_slider_time);
    return music;
}

static void music_delete(lv_obj_t* obj)
{
    lv_timer_del(music_slider_time);
    lv_obj_del(obj);
    lv_style_reset(&style_btn);
    lv_style_reset(&style_btn1);
    lv_style_reset(&style_control);
    lv_style_reset(&style_slider_main);
    lv_style_reset(&style_cont);
    close(fd_fifo);
    pthread_cancel(music_r_id);
    pthread_cancel(music_w_id);
    kill(mplayer_pid, 9);
    pre_btn_id = 0;
    start = 0;
    ready = 0;
    pthread_mutex_destroy(&mplayer_mutex);
    pthread_cond_destroy(&mplayer_cond);
    time_start = 0;
    time_end = 0;
    btn_exchange = 0;
    btn_music_id = 0;
    music_num = 0;
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

static void send_cmd(char *buff)
{
    char buf[256];
    strcpy(buf,buff);
	if(write(fd_fifo,buf,strlen(buf))!=strlen(buf))
			perror("write");
	if(strstr(buf, "loadfile") != NULL){
		while(1)
		{
			read(fd_pipe[0],buf,sizeof(buf));
			if(strstr(buf,"...") != NULL) break;
			memset(buf, '\0', sizeof(buf));
		}
		memset(buf, '\0', sizeof(buf));
	}
	if((strcmp(buf, "pause\n")!=0) && (strstr(buf, "loadfile") == NULL)){;
		pthread_mutex_lock(&mplayer_mutex);
		ready = 1;
		pthread_mutex_unlock(&mplayer_mutex);
		pthread_cond_signal(&mplayer_cond);//唤醒接受线程
	}
}

static void *write_pthread(void *arg)
{   
    send_cmd("volume 0 1\n");
    send_cmd("loadlist /home/cxj/Desktop/fish_pi/music/music.lst\n");
	while(start == 0);
	send_cmd("get_time_length\n");
	send_cmd("volume 100 1\n");
	send_cmd("pause\n");
	char buf[100];
	while(1)
	{
		// printf("please input you cmd:");
		fflush(stdout);
		fgets(buf,sizeof(buf),stdin);		//从标准输入获取数据
		printf("*%s",buf);	
		send_cmd(buf);		
	}
    pthread_exit((void *)0);
}

static void *read_pthread(void *arg)
{
	char buf[256];
    char tmp[10];
    char *delim = "=";
    char *p = NULL;
	close(fd_pipe[1]);
	while(1)
	{
		read(fd_pipe[0],buf,sizeof(buf));//过滤掉加载歌曲的打印
		if(strstr(buf,"...") != NULL) break;
		memset(buf, '\0', sizeof(buf));
	}
	memset(buf, '\0', sizeof(buf));
    while(1)
	{
		read(fd_pipe[0],buf,sizeof(buf));//过滤掉加载歌曲的打印
		if(strstr(buf,"...") != NULL) break;
		memset(buf, '\0', sizeof(buf));
	}
	memset(buf, '\0', sizeof(buf));
	start = 1;
	while(1)
	{
		pthread_mutex_lock(&mplayer_mutex);
		while(ready == 0){
			pthread_cond_wait(&mplayer_cond, &mplayer_mutex);
		}
        ready = 0;
		pthread_mutex_unlock(&mplayer_mutex);
		read(fd_pipe[0],buf,sizeof(buf));	//从无名管道的写端读取信息打印在屏幕上
        //printf("read:::%s\n", buf);
        if(strstr(buf, "ANS") != NULL){
            p = strtok(buf, delim);
            if(strcmp(p, "ANS_LENGTH") == 0){
                p = strtok(NULL, delim);
                time_end = atof(p);
                sprintf(tmp,"%d%d:%d%d", time_end/60/10, time_end/60%10, 
                                    time_end%60/10, time_end%60%10);
                pthread_mutex_lock(&lvgl_mutex);
                lv_slider_set_range(slider_time, 0 , (int)time_end);
                lv_label_set_text(lable_end, tmp);
                pthread_mutex_unlock(&lvgl_mutex);
                memset(tmp, '\0', sizeof(tmp));
            }
        }
		memset(buf, '\0', sizeof(buf));
	}
    pthread_exit((void *)0);
}
//ANS_LENGTH
static void music_hand(void)
{
	pid_t pid;
	
	unlink("/tmp/my_fifo");					//如果明明管道存在，则先删除
	mkfifo("/tmp/my_fifo",O_CREAT|0666);

    pthread_mutex_init(&mplayer_mutex, NULL);
	pthread_cond_init(&mplayer_cond, NULL);

    fd_fifo=open("/tmp/my_fifo",O_RDWR);
    if(fd_fifo<0) perror("open");
	
	if (pipe(fd_pipe)<0 )					//创建无名管道
	{
		perror("pipe error\n");
		exit(-1);
	}
 
	pid=fork();
	if(pid<0)
	{
		perror("fork");
	}
	if(pid==0)								//子进程播放mplayer
	{
		close(fd_pipe[0]);
		dup2(fd_pipe[1],STDOUT_FILENO);					//将子进程的标准输出重定向到管道的写端
        //system("mplayer -slave -quiet -input file=/tmp/my_fifo /home/cxj/Desktop/fish_pi/music/稻香.mp3");
		execlp("mplayer","mplayer","-slave","-quiet","-loop","0","-input","file=/tmp/my_fifo","/home/cxj/Desktop/fish_pi/music/稻香.mp3",NULL);
	}
	else
	{
        mplayer_pid = pid;
			
		pthread_create(&music_w_id, NULL, write_pthread, NULL);
        pthread_detach(music_w_id);
        pthread_create(&music_r_id, NULL, read_pthread, NULL);
        pthread_detach(music_r_id);
	}
}


void music_cmd_handle(void)
{
    struct cmd_data cmd;
    music_cmd_read(cmd_head, &cmd);
    if(strcmp(cmd.cmd_name.name,"music create") == 0){
        pthread_mutex_lock(&lvgl_mutex);
        page_delete(page_head, cmd.cmd_info.info);
        page_create(page_head, "music");
        pthread_mutex_unlock(&lvgl_mutex);
    }else if(strcmp(cmd.cmd_name.name,"music name") == 0){
        char tmp[50];
        char name_buf[50];
        char *delim = " ";
        memset(name_buf, '\0', sizeof(name_buf));
        char *p = strtok(cmd.cmd_info.info, delim);
        while(1){
            strcat(name_buf,p);
            p = strtok(NULL, delim);
            if(p == NULL) break;
            strcat(name_buf,"\\ ");
        }
        send_cmd("volume 0 1\n");
        sprintf(cmd_buf, "loadfile /home/cxj/Desktop/fish_pi/music/%s\n", name_buf);
        send_cmd(cmd_buf);
        send_cmd("get_time_length\n");
        sprintf(tmp, "volume %d 1\n", voice);
        send_cmd(tmp);
        pthread_mutex_lock(&lvgl_mutex);
        lv_obj_t * obj = lv_obj_get_child(btn1, 0);
        if(lv_img_get_src(obj) == &image17){//根据按键判断切歌要不要播放
            send_cmd("pause\n");
        }
        time_start = 0;
        sprintf(tmp,"%d%d:%d%d", time_start/60/10, time_start/60%10, 
                                time_start%60/10, time_start%60%10);
        lv_label_set_text(lable_start, tmp);
        pthread_mutex_unlock(&lvgl_mutex);
    }else if(strcmp(cmd.cmd_name.name,"music control") == 0){
        if(write(fd_fifo,cmd.cmd_info.info,strlen(cmd.cmd_info.info))!=strlen(cmd.cmd_info.info))
			perror("write");  
    }else if(strcmp(cmd.cmd_name.name,"music voice") == 0){
        char tmp[60];
        sprintf(tmp, "volume %s 1\n", cmd.cmd_info.info);
        send_cmd(tmp);
    }else if(strcmp(cmd.cmd_name.name,"music time") == 0){
        char tmp[60];
        sprintf(tmp, "seek %s 2\n", cmd.cmd_info.info);
        send_cmd(tmp);
    }
}

#include "lvgl/lvgl.h"
//#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "ui/ui.h"

#define DISP_BUF_SIZE (240 * 240)


int main(void)
{
    pthread_t lvgl_id;
    pthread_t cmd_id;

    mutex_init();
    pthread_create(&lvgl_id, NULL, lvgl_start, NULL);
    //pthread_detach(lvgl_id);
    pthread_create(&cmd_id, NULL, cmd_handle, NULL);
    //pthread_detach(cmd_id);
    
    pthread_join(lvgl_id, NULL);
    pthread_join(cmd_id, NULL);
    return 0;
    // while(1){
    //     sleep(1);
    // }
    // pthread_exit((void *)0);
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

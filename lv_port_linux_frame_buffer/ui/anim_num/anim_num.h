#include "lvgl/lvgl.h"
#include <stdlib.h>

#define DOT_NUM 20//点数





struct anim_num{
    lv_obj_t* obj;//线对象
    lv_point_t * dot;//点集表
    uint8_t pre_num;//上一个数字
    uint8_t now_num;//现在数字
    unsigned char time;//缩小倍数
    lv_style_t style_line;//样式
};


//struct anim_num*  anim_num_create (lv_obj_t* par=lv_scr_act(),int x=0,int y=0,unsigned char time=4,lv_color_t col=lv_palette_main(LV_PALETTE_BLUE),unsigned char wid=8);//创建一个数字，返回为该对象的编号
struct anim_num*  anim_num_create (lv_obj_t* par,int x,int y,unsigned char time,lv_color_t col,unsigned char wid);//创建一个数字，返回为该对象的编号
void anim_num_del(struct anim_num* del_obj);
//void disp(struct anim_num* anim_obj,unsigned int t=500);//改变显示数字。数字(10为归位)与动画时长
void anim_num_disp(struct anim_num* anim_obj,unsigned char num,unsigned int t);//改变显示数字。数字(10为归位)与动画时长
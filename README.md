## 硬件

主要参考网上的开源大佬，Orange Pi和Nano Pi。  
注意H3的上电时序可以看芯片datashee  
DDR注意等长，单端和差分阻抗，参考面  
（最近在忙其他的目前只有摄像头驱动没调出来原理图看起来是没有错）

## 软件

uboot和kernel 主要参考Orange Pi和Nano Pi进行修改  
文件系统用ubuntu22.04base，没有用buildroot主要是移植一些软件没有apt方便，又对启动速度之类的没有要求  

***
具体代码在lv_port_linux_frame_buffer/ui   
模拟器lv_sim_vscode_sdl可以直接运行    
简单过程：
通过创建一个链表来控制ui页面切换一个cmd链表进行控制
![](C:\Users\CXJ\Desktop\linux学习\捕获.PNG)

1. UI选择用LVGL（主要是其他的不会）  
2. time是用lvgl定时器加date命令实现  
3. wifi实现有三个方法：   
     - popen这样速度会有点慢  
     - 建立管道重定向标准输入输出 wpa_cli  
     - 用wpa_supplicant的接口(想折腾下就用了这个)  
4. music直接选用地球上最强的播放器mplayer，利用slave模式重定向输入输出  
5. video当然也选择mplayer  
6. 语言识别计划用开源的ASRT跑不起来就用百度的api  
7. 摄像头可能用opencv跑个简单识别（如果驱动不出现意外）  
8. cpu资源显示  

***

|     功能     | 状态 |
| :----------: | :--: |
|     LVGL     |  ✅   |
|   时间日期   |  ✅   |
|   wifi功能   |  ✅   |
|  音乐播放器  |  ✅   |
|  视频播放器  |  ❌   |
|   语音控制   |  ❌   |
|  摄像头识别  |  ❌   |
| 芯片资源显示 |  ❌   |

## 致谢

https://whycan.com/t_7929.html  
https://github.com/caichengwu/GwCard  
http://www.friendlyelec.com.cn/download.asp  
https://github.com/orangepi-xunlong
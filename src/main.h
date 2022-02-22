#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

#define server_ip "bemfa.com"   //巴法云服务器地址默认即可
#define server_port "8344"      //服务器端口，tcp创客云端口8344

#define MAX_PACKETSIZE 512      //最大字节数
#define KEEPALIVEATIME 30*1000  //设置心跳值30s

#define RED_LED_Pin 14          //红灯
#define BLUE_LED_Pin 12         //蓝色灯
#define PLUG_Pin 5              //继电器
#define BUTTON_Pin 4            //开关

// 微信推送相关
// const int delaytime = 0;                                          //为了防止被设备“骚扰”，可设置贤者时间，单位是秒，如果设置了该值，在该时间内不会发消息到微信，设置为0立即推送。
// String ApiUrl = "https://go.bemfa.com/v1/sendwechat";           //默认 api 网址
// static String ApiUrl = "http://api.bemfa.com/api/wechat/v1/"; 
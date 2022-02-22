#pragma once
#include "main.h"
#include <WiFiUDP.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Ticker.h>

#define MAGIC_NUMBER 0xAA //判断是否配网

struct config_type
{
    char stassid[32];
    char stapsw[16];
    char cuid[40];
    char ctopic[32];
    uint8_t reboot;
    uint8_t magic;
};

void loadConfig();      // 加载配置
void saveConfig();      // 保存WIFI信息
void restoreFactory();  // 恢复出厂设置
void apConfig(String mac);  // 开始ap配网
void delayRestart(float t);    //延时启动
bool AutoConfig();      // 自动连接
void initWifi();        // wifi初始化
void wifiDog();         // wifi狗
#include "connect_wifi.h"


//自动连接wifi
bool AutoConfig()
{
    WiFi.begin();
    //如果觉得时间太长可改
    for (int i = 0; i < 20; i++)
    {
        int wstatus = WiFi.status();
        if (wstatus == WL_CONNECTED)
        {
            Serial.println("WIFI SmartConfig Success");
            Serial.printf("SSID:%s", WiFi.SSID().c_str());
            Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
            Serial.print("LocalIP:");
            Serial.print(WiFi.localIP());
            Serial.print(" ,GateIP:");
            Serial.println(WiFi.gatewayIP());
            return true;
        }
        else
        {
            Serial.print("WIFI AutoConfig Waiting......");
            Serial.println(wstatus);
            // flash LED
            digitalWrite(BLUE_LED_Pin,LOW);
            delay(500);                     
            digitalWrite(BLUE_LED_Pin,HIGH);
            delay(500);
        }
    }
    Serial.println("WIFI AutoConfig Faild!" );
    return false;
}


//智能连接wifi
void SmartConfig()
{
    WiFi.mode(WIFI_STA);
    Serial.println("\r\nWait for Smartconfig...");
    WiFi.beginSmartConfig();
    while (1)
    {
        // flash LED
        digitalWrite(BLUE_LED_Pin,LOW);
        delay(100);                     // wait for a second
        digitalWrite(BLUE_LED_Pin,HIGH);
        delay(100);
        Serial.print(".");
        if (WiFi.smartConfigDone())
        {
            Serial.println("SmartConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
            Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
            break;
        }
    }
}


// 初始化wifi
void initWifi(){
    if (!AutoConfig())
    {
        SmartConfig();
    }
}


// Wifi看门狗
void wifiDog(){
    int wstatus = WiFi.status();
    if (wstatus != WL_CONNECTED){
        Serial.println("Wifi Disconnected , Reconnect Wifi...");
        initWifi();
    }
}
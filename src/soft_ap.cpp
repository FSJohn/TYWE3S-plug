#include "soft_ap.h"

/*
 * 初始化变量
*/
char config_flag = 0;//判断是否配网
String type = "001"; //设备类型，001插座设备，002灯类设备，003风扇设备

extern config_type config;      


/*
 * 智能连接wifi
*/
bool AutoConfig()
{
    WiFi.disconnect();//断开连接
    WiFi.mode(WIFI_STA);//STA模式
    WiFi.begin(config.stassid, config.stapsw);//连接路由器
    //如果觉得时间太长可改
    for (int i = 0; i < 20; i++)
    {
        int wstatus = WiFi.status();
        if (wstatus == WL_CONNECTED)
        {
            Serial.println("\nWIFI SmartConfig Success");
            Serial.printf("SSID:%s", WiFi.SSID().c_str());
            Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
            Serial.print("LocalIP:");
            Serial.print(WiFi.localIP());
            Serial.print(",GateIP:");
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

/*
 * 从EEPROM加载参数
*/
void loadConfig()
{
    uint8_t *p = (uint8_t*)(&config);
    uint8_t mac[6];
    Serial.println("LoadConfig.......");
    WiFi.macAddress(mac);
    EEPROM.begin(sizeof(config));
    for (unsigned int i = 0; i < sizeof(config); i++)
    {
        *(p + i) = EEPROM.read(i);
    }
    config.reboot = config.reboot + 1;
    Serial.printf("config.reboot:%d\n",config.reboot);
    if(config.reboot>=4){
        Serial.println("restoreFactory!");
        restoreFactory();
    }
    if(config.magic != 0xAA){
        config_flag = 1;
    }
    EEPROM.begin(sizeof(config));
    for (unsigned int i = 0; i < sizeof(config); i++){
        EEPROM.write(i, *(p + i));
    }
    EEPROM.commit();
    delay(2000);
    EEPROM.begin(sizeof(config));
    config.reboot = 0;
    for (unsigned int i = 0; i < sizeof(config); i++){
        EEPROM.write(i, *(p + i));
    }
    EEPROM.commit();

    Serial.println("\n--Config Start--");
    Serial.print("SSID:");
    Serial.println(config.stassid);
    Serial.print("PASSWD:");
    Serial.println(config.stapsw);
    Serial.print("UID:");
    Serial.println(config.cuid);
    Serial.print("topic:");
    Serial.println(config.ctopic);
    Serial.println("--Config End--");
}


/* 
 * 恢复出厂设置
*/
void restoreFactory()
{
    Serial.println("\r\n Restore Factory....... ");
    config.magic = 0x00;
    strcpy(config.stassid, "");
    strcpy(config.stapsw, "");
    strcpy(config.cuid, "");
    strcpy(config.ctopic, "");
    config.magic = 0x00;
    saveConfig();
    //闪烁红灯
    for(int i=0;i<5;i++){
        // flash LED
        digitalWrite(RED_LED_Pin,LOW);
        delay(250);                     
        digitalWrite(RED_LED_Pin,HIGH);
        delay(250);
    }
    delayRestart(1);
    while (1) {
        ESP.wdtFeed();
        delay(100);
    }
}

/*
 * 保存WIFI信息
*/
void saveConfig()
{
    config.reboot = 0;
    EEPROM.begin(2018);
    uint8_t *p = (uint8_t*)(&config);
    for (unsigned int i = 0; i < sizeof(config); i++)
    {
        EEPROM.write(i, *(p + i));
    }
    EEPROM.commit();
}

Ticker delayTimer;
void delayRestart(float t) {
    delayTimer.attach(t, []() {
        ESP.restart();
    });
}

/*
 * 开始配网
*/
void apConfig(String mac){
    WiFiUDP Udp;
    char packetBuffer[255];     //发送数据包

    Serial.println("Ap Config...");
    if(config_flag == 1){
        WiFi.disconnect();
        WiFi.softAP("ChaZuo_"+mac);
        Udp.begin(8266);
        Serial.println("Started Ap Config...");
    }
    String topic = mac+type;
    while(config_flag){//如果未配网，开启AP配网，并接收配网信息
        // flash LED
        digitalWrite(BLUE_LED_Pin,LOW);
        delay(250);                     
        digitalWrite(BLUE_LED_Pin,HIGH);
        delay(250);
        Serial.print(".");
        int packetSize = Udp.parsePacket();
        if (packetSize) {
            Serial.print("Received packet of size ");
            Serial.println(packetSize);
            Serial.print("From ");
            IPAddress remoteIp = Udp.remoteIP();
            Serial.print(remoteIp);
            Serial.print(", port ");
            Serial.println(Udp.remotePort());

            int len = Udp.read(packetBuffer, 255);
            if (len > 0) {
                packetBuffer[len] = 0;
            }
            Serial.println("Contents:");
            Serial.println(packetBuffer);
            StaticJsonDocument<200> doc;

            DeserializationError error = deserializeJson(doc, packetBuffer);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }
            int cmdType = doc["cmdType"].as<int>();;
            const char* ssid = doc["ssid"];
            const char* password = doc["password"];
            const char* token = doc["token"];
            //const char* ctopic = doc["topic"];
            Serial.println(cmdType);
            Serial.println(ssid);
            Serial.println(password);
            Serial.println(token);
            if (cmdType == 1) {
                config_flag = 0;
                strcpy(config.stassid, ssid);
                strcpy(config.stapsw, password);
                strcpy(config.cuid, token);
                strcpy(config.ctopic, topic.c_str());
                config.reboot = 0;
                config.magic = 0xAA;
                saveConfig();
                //收到信息，并回复
                String  ReplyBuffer = "{\"cmdType\":2,\"productId\":\""+topic+"\",\"deviceName\":\"插座\",\"protoVersion\":\"3.0\"}";
                Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
                Udp.write(ReplyBuffer.c_str());
                Udp.endPacket();
                WiFi.softAPdisconnect(true);
            }
        }
  }
}

// 初始化wifi
void initWifi(){
    String topic = WiFi.macAddress().substring(8);//取mac地址做主题用
    topic.replace(":", "");//去掉:号
    loadConfig();//加载存储的数据
    apConfig(topic);//加载ap
    while (!AutoConfig()){//检查是否连接成功,失败则重新配网
        config_flag = 1;
        apConfig(topic);//加载ap
    }
    Serial.println("\n--Config Start--");
    Serial.print("SSID:");
    Serial.println(config.stassid);
    Serial.print("PASSWD:");
    Serial.println(config.stapsw);
    Serial.print("UID:");
    Serial.println(config.cuid);
    Serial.print("topic:");
    Serial.println(config.ctopic);
    Serial.println("--Config End--");
}

// Wifi看门狗
void wifiDog(){
    int wstatus = WiFi.status();
    if (wstatus != WL_CONNECTED){
        Serial.println("Wifi Disconnected , Reconnect Wifi...");
            // flash LED
            digitalWrite(BLUE_LED_Pin,LOW);
            delay(500);                     
            digitalWrite(BLUE_LED_Pin,HIGH);
            delay(500);
    }
}

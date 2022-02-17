#include "main.h"
#include "led.h"
#include "connect_wifi.h"


String otaUrl = "http://bin.bemfa.com/b/3BcYTA5YmE2NjhkZDIzNDNkYWE5N2VkMDVlNjFjYWVlYjQ=chazuo001.bin";//固件链接，在巴法云控制台复制、粘贴到这里即可

//热更新
void updateBin(){
    Serial.println("start update");    
    WiFiClient UpdateClient;
    t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateClient, otaUrl);
    switch(ret) {
        case HTTP_UPDATE_FAILED:      //当升级失败
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            Serial.println("[update] Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:  //当无升级
            Serial.println("[update] Update no Update.");
            break;
        case HTTP_UPDATE_OK:         //当升级成功
            Serial.println("[update] Update ok.");
            break;
    }
}

//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";       //初始化字符串，用于接收服务器发来的数据
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;         //心跳
unsigned long preTCPStartTick = 0;      //连接
bool preTCPConnected = false;           //TCP预连接?

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);


//发送数据到TCP服务器
void sendtoTCPServer(String p){
    if (!TCPclient.connected()) 
    {
        Serial.println("Client is not readly");
        return;
    }
    TCPclient.print(p);
}

// 初始化和服务器建立连接
void startTCPClient(){
    if(TCPclient.connect(server_ip, atoi(server_port))){
        Serial.print("\nConnected to server:");
        Serial.printf("%s:%d\r\n",server_ip,atoi(server_port));
        
        String tcpTemp="";  //初始化字符串
        tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n"; //构建订阅指令
        sendtoTCPServer(tcpTemp); //发送订阅指令
        tcpTemp.clear();//清空

        /*
        //如果需要订阅多个主题，可再次发送订阅指令
        tcpTemp = "cmd=1&uid="+UID+"&topic="+主题2+"\r\n"; //构建订阅指令
        sendtoTCPServer(tcpTemp); //发送订阅指令
        tcpTemp="";//清空
        */
        
        preTCPConnected = true;
        preHeartTick = millis();
        TCPclient.setNoDelay(true);
    }
    else{
        Serial.print("Failed connected to server:");
        Serial.println(server_ip);
        TCPclient.stop();
        preTCPConnected = false;
    }
    preTCPStartTick = millis();
}

/*
  *检查数据，发送心跳
*/
void doTCPClientTick(){
    if(WiFi.status() != WL_CONNECTED) return;//检查是否断开，断开后重连
    if(!TCPclient.connected()) {//断开重连
        if(preTCPConnected == true){
            preTCPConnected = false;
            preTCPStartTick = millis();
            Serial.println();
            Serial.println("TCP Client disconnected.");
            TCPclient.stop();
        }
        // else if(millis() - preTCPStartTick > 1*1000)//重新连接
        else if(millis() - preTCPStartTick > 250){
            startTCPClient();        //重新连接
        }
    }else{
        if(TCPclient.available()){
            //收数据
            char c = TCPclient.read();
            TcpClient_Buff += c;
            TcpClient_BuffIndex++;
            TcpClient_preTick = millis();
            
            if(TcpClient_BuffIndex >= MAX_PACKETSIZE - 1){
                TcpClient_BuffIndex = MAX_PACKETSIZE -2;
                TcpClient_preTick = TcpClient_preTick - 200;
            }
            preHeartTick = millis();
        }
        if(millis() - preHeartTick >= KEEPALIVEATIME){//保持心跳
            preHeartTick = millis();
            Serial.println("Keep alive");
            sendtoTCPServer("ping\r\n"); //发送心跳，指令需\r\n结尾，详见接入文档介绍
        }
    }
    // 数据处理
    if((TcpClient_Buff.length() > 0 && (millis() - TcpClient_preTick >= 200)))
    {
        String getTopic = "";
        String getMsg = "";

        TCPclient.flush();
        Serial.print("Rev string: ");
        TcpClient_Buff.trim();              //去掉首位空格
        Serial.println(TcpClient_Buff);     //打印接收到的消息

        if(TcpClient_Buff.length() > 15){
            // 注意 TcpClient_Buff 只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";
            // 此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=light002&msg=off
            int topicIndex = TcpClient_Buff.indexOf("&topic=")+7;   
            //c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
            int msgIndex = TcpClient_Buff.indexOf("&msg=");
            //c语言字符串查找，查找&msg=位置
            getTopic = TcpClient_Buff.substring(topicIndex,msgIndex);
            //c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
            getMsg = TcpClient_Buff.substring(msgIndex+5);
            //c语言字符串截取，截取到消息
            Serial.print("---topic:");
            Serial.println(getTopic); //打印截取到的主题值
            Serial.print("-----msg:");
            Serial.println(getMsg);   //打印截取到的消息值
        }
        if(getMsg  == "on"){       //如果收到指令on==打开灯
            turnOnLed();
        }else if(getMsg == "off"){ //如果收到指令off==关闭灯
            turnOffLed();
        }else if(getMsg == "update"){  //如果收到指令update
            updateBin();//执行升级函数
        }

        TcpClient_Buff.clear(); //清空
        TcpClient_BuffIndex = 0;
    }
}



// 初始化，相当于main 函数
void setup() {
    //初始化串口
    Serial.begin(115200);

    //接口初始化
    pinMode(RED_LED_Pin,OUTPUT);
    pinMode(BLUE_LED_Pin,OUTPUT);
    pinMode(PLUG_Pin,OUTPUT);
    pinMode(BUTTON_Pin,INPUT);

    //默认上电，开启灯
    digitalWrite(RED_LED_Pin,LOW);  //低电平开灯
    digitalWrite(PLUG_Pin,HIGH);        //开启继电器

    //初始网络连接
    initWifi();
}

//主循环
void loop() {
    wifiDog();    // 断线重连
    doTCPClientTick();    //TCP监听
    buttonLed();    //物理按钮
}

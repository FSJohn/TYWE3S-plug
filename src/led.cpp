#include "led.h"


//打开灯泡
void turnOnLed(){
  Serial.println("Turn ON");
  digitalWrite(BLUE_LED_Pin,LOW);
  //Serial.println("LEd down");
  digitalWrite(PLUG_Pin,HIGH);
  //Serial.println("plug down");
//   doHttpStick(msgOn); //发送打开通知
}


//关闭灯泡
void turnOffLed(){
    Serial.println("Turn OFF");
    digitalWrite(BLUE_LED_Pin,HIGH);
    digitalWrite(PLUG_Pin,LOW);
    // doHttpStick(msgOff); //发送关闭通知   
}


//按钮执行关闭或打开
void buttonLed(){
    if(digitalRead(BUTTON_Pin) == LOW)
    {
        delay(10);
        if(digitalRead(BUTTON_Pin) == LOW)
        {
            digitalWrite(BLUE_LED_Pin,!digitalRead(BLUE_LED_Pin));
            digitalWrite(PLUG_Pin,!digitalRead(PLUG_Pin));
            while(digitalRead(BUTTON_Pin) == LOW);
    }
  }
}

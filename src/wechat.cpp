// #include "wechat.h"

// //******微信消息推送函数********//
// void doHttpStick(String msg,HTTPClient http,String UID,int delaytime,WiFiClient TCPclient,String ApiUrl){  //微信消息推送函数
//   String postData;
//   //Post Data
//   postData = "uid="+UID+"&type="+type+"&time="+delaytime+"&device="+device+"&msg="+msg;
//   http.begin(TCPclient,ApiUrl);              //Specify request destination
//   http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
//   int httpCode = http.POST(postData);   //Send the request
//   String payload = http.getString();    //Get the response payload
//   Serial.println(httpCode);     //Print HTTP return code
//   Serial.println(payload);      //Print request response payload
//   http.end();                   //Close connection
//   Serial.println("send success");  
//   }
// //=======================================================================
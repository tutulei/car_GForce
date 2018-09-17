#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <gForceAdapterESP8266.h>


int FIST = 0;
int SPREAD = 0;
int WAVEIN = 0;
int WAVEOUT = 0;
int PINCH = 0;
int SHOOT = 0;


const char* ssid = "GForce_f";//连接的路由器的名字
const char* password = "llfhaoshuai";//连接的路由器的密码
const char* mqtt_server = "192.168.137.1";//服务器的地址 iot.eclipse.org是开源服务器


GForceAdapter gforce = GForceAdapter(&Serial);

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;//存放时间的变量
char msg[17];//存放要发的数据

void setup_wifi() {//自动连WIFI接入网络
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {//用于接收服务器接收的数据

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);//串口打印出接收到的数据
  }
  Serial.println();//换行

}

void reconnect() {//等待，直到连接上服务器
  while (!client.connected()) {//如果没有连接上
    if (client.connect("hsghgjyjsdvna") + random(999999999)) { //接入时的用户名，尽量取一个很不常用的用户名
      client.subscribe("666");//接收外来的数据时的intopic
    } else {
      Serial.print("failed, rc=");//连接失败
      Serial.print(client.state());//重新连接
      Serial.println(" try again in 5 seconds");//延时5秒后重新连接
      delay(5000);
    }
  }
}

void setup() {//初始化程序，只运行一遍
  gforce.Init();
  setup_wifi();//自动连WIFI接入网络
  client.setServer(mqtt_server, 1883);//1883为端口号
  client.setCallback(callback); //用于接收服务器接收的数据
}



void loop() {//主循环
  reconnect();//确保连上服务器，否则一直等待。
  client.loop();//MUC接收数据的主循环函数。
  GF();
  encodeJson();
}


void encodeJson() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root1 = jsonBuffer.createObject();
  root1["FIST"] = FIST;
  root1["SPREAD"] = SPREAD;
  root1["WAVEIN"] = WAVEIN;
  root1["WAVEOUT"] = WAVEOUT;
  root1["PINCH"] = PINCH;
  root1["SHOOT"] = SHOOT;
  if (SHOOT == 1) {
   snprintf(msg,75,"{\"D\":1,\"S\":1024}");
  }
  else if (SPREAD == 1) {
   snprintf(msg,75,"{\"D\":0,\"S\":1024}");
  }
  else if (WAVEIN == 1) {
   snprintf(msg,75,"{\"D\":3,\"S\":1024}");
  }
  else if (WAVEOUT == 1) {
   snprintf(msg,75,"{\"D\":4,\"S\":1024}");
  }
  else if (FIST == 1) {
   snprintf(msg,75,"{\"D\":2,\"S\":1024}");
  }
  else {
   snprintf(msg,75,"{\"D\":0,\"S\":1024}");
  }
}

void GF() {
  GF_Data gForceData;
  if (GF_OK == gforce.GetGForceData(&gForceData)) {
    GF_Gesture gesture;
    switch (gForceData.type) {
      case GF_Data::QUATERNION:
        break;
      case GF_Data::GESTURE:
        gesture = gForceData.value.gesture;
        if (gesture == GF_FIST) {
          FIST = 1;
          SPREAD = 0;
          WAVEIN = 0;
          WAVEOUT = 0;
          PINCH = 0;
          SHOOT = 0;
          encodeJson();
          client.publish("GForce_f", msg);
        } else if (gesture == GF_SPREAD) {
          FIST = 0;
          SPREAD = 1;
          WAVEIN = 0;
          WAVEOUT = 0;
          PINCH = 0;
          SHOOT = 0;
          encodeJson();
          client.publish("GForce_f", msg);
        } else if (gesture == GF_WAVEIN) {
          FIST = 0;
          SPREAD = 0;
          WAVEIN = 1;
          WAVEOUT = 0;
          PINCH = 0;
          SHOOT = 0;
          encodeJson();
          client.publish("GForce_f", msg);
        } else if (gesture == GF_WAVEOUT) {
          FIST = 0;
          SPREAD = 0;
          WAVEIN = 0;
          WAVEOUT = 1;
          PINCH = 0;
          SHOOT = 0;
          encodeJson();
          client.publish("GForce_f", msg);
        } else if (gesture == GF_PINCH) {
          FIST = 0;
          SPREAD = 0;
          WAVEIN = 0;
          WAVEOUT = 0;
          PINCH = 1;
          SHOOT = 0;
          encodeJson();
          client.publish("GForce_f", msg);
        } else if (gesture == GF_SHOOT) {
          FIST = 0;
          SPREAD = 0;
          WAVEIN = 0;
          WAVEOUT = 0;
          PINCH = 0;
          SHOOT = 1;
          encodeJson();
          client.publish("GForce_f", msg);
        } else if (gesture == GF_RELEASE) {
          FIST = 0;
          SPREAD = 0;
          WAVEIN = 0;
          WAVEOUT = 0;
          PINCH = 0;
          SHOOT = 0;
          encodeJson();
          client.publish("GForce_f", msg);
        } else if (gesture == GF_UNKNOWN) {
          FIST = 1;
          SPREAD = 1;
          WAVEIN = 1;
          WAVEOUT = 1;
          
          PINCH = 1;
          SHOOT = 1;
          encodeJson();
          client.publish("GForce_f", msg);
        }
        break;
      default:
        break;
    }
  }
}

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

const char* ssid = "Ubilabs";//WiFi名称
const char* password = "googleiot";//WiFi密码
const char* mqtt_server = "47.106.69.63";//MQTT broker地址


//全局变量区域上界
int op=0;//500毫秒计时器每500毫秒自加一
int wheel=2;//wheel=0表示两轮电压相同，左轮转得慢，wheel=1则是右轮转得慢
long last[5]={0};//存放时间，与计时器配合，代替delay效果
int pulse_listen[2]={0};
int pulse_change[4]={0};
int L_speed[4]={0};
int R_speed[4]={0};
int count_dif[5]={0};//0放总偏差，1放50毫秒偏差
int execute_straight=2;
int command_s=0;
int PWM_temp[2]={0};
int PWM[2]={0};//PWM[0]为L的PWM值，调直函数用到。
int DT=50;//测速间隔（毫秒）
int Dcount[2]={0};
int adjust_count=0;
double PID[2]={0.5,1};
//调试变量

int Rtt=0;
int Ltt=0;
int Dtt=0;
int Stt=0;


//全局变量区域下界

WiFiClient espClient;
PubSubClient client(espClient);

int OTA=0;
int OTAS=0;
long lastMsg = 0;//存放时间的变量 
char msg[200];//存放要发的数据
String load;


void setup_wifi() {//自动连WIFI接入网络
  delay(10);
    WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}



void reconnect() {//等待，直到连接上服务器
  while (!client.connected()) {//如果没有连接上
    int randnum = random(0, 999); 
    if (client.connect("OTADEMO"+randnum)) {//接入时的用户名，尽量取一个很不常用的用户名
      client.subscribe("ImAndroid");
    } else {
      Serial.print("failed, rc=");//连接失败
      Serial.print(client.state());//重新连接
      Serial.println(" try again in 3 seconds");//延时3秒后重新连接
      delay(3000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {//用于接收服务器接收的数据
  load="";
  for (int i = 0; i < length; i++) {
      load +=(char)payload[i];//串口打印出接收到的数据
  }
   decodeJson();
}

void  decodeJson() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(load);
   OTA = root["OTA"];
   OTAS =OTA;
   //接收数据json处理区上界
   int L=root["L"];
   int R=root["R"];
   int S=root["S"];
   int D=root["D"];
   command_s=S;
   Ltt=L;
   Rtt=R;
   Stt=S;
   Dtt=D;
   translate(D,S,L,R);
   //接收数据json处理区下界
}


void translate(int D,int S,int L,int R){//处理接收到的指令，调用电机执行函数
   if(S<0) {  S=0-S; }//当指令中速度为负数
   if(D==0){
   execute_straight=2;
   ept_values(1);
   execute(1,1,0,0);
    DT=50;//设定测速周期为50毫秒一次返回
   }
   else if(D==1){
    ept_values(1);
    execute_straight=1;
     DT=50;
   }
   else if(D==2){
    ept_values(1);
    execute_straight=0;
     DT=50;
   }
   else if(D==3){
    execute_straight=2;
    execute(0,1,S,S);
     DT=50;
     ept_values(1);
   }
   else if(D==4){
    execute_straight=2;
    execute(1,0,S,S);
     DT=50;
     ept_values(1);
   }
  else if(D==5){
     DT=50;
    execute_straight=2;
    bool execute_l=1;
    bool execute_r=1;
    if(L<0)
    {
    L=-L;
    execute_l=0;
    }
    if(R<0)
    {
      execute_r=0;
      R=-R;
      }
    execute(execute_l,execute_r,L,R);
   }
}

void execute(bool execute_l,bool execute_r,int l_speed,int r_speed)//电机执行函数，execute_l为1，左电机前进，为0则退；execute_r同理。l_speed表示左电机的PWM值；r_speed同理。
{
  digitalWrite(D1,0+execute_r);
  digitalWrite(D0,!(0+execute_r));
  analogWrite(D3,r_speed);
  digitalWrite(D2,0+execute_l);
  digitalWrite(D4,!(0+execute_l));
  analogWrite(D5,l_speed);
}

void OTAsetup(){//OTA准备
   if(OTAS){
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  OTAS=0;
   }
}


void setup() {
  //setup代码区域上界
  Serial.begin(9600);
  pinMode(D0,OUTPUT); 
  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(D3,OUTPUT);
  pinMode(D4,OUTPUT);
  pinMode(D6,INPUT);
  pinMode(D5,OUTPUT);
  pinMode(D7,INPUT);

  //setup代码区域下界

   setup_wifi();//自动连WIFI接入网络
  client.setServer(mqtt_server, 1883);//1883为端口号
  client.setCallback(callback); //用于接收服务器接收的数据
}

void loop() {
        if(OTA){//如果解析到OTA指令
          OTAsetup();//进入OTA模式
        ArduinoOTA.handle();
       }
       else{
        reconnect();//确保连上服务器，否则一直等待。
        client.loop();//MUC接收数据的主循环函数。
        //loop代码上界

       if(op<=6){//测试电机物理特性
        seek_wheel();//通过原地旋转寻找两电机在物理上的特性
        }
        pulse_num();//测两个码盘的脉冲
        straight(execute_straight);//走直算法

        //loop代码下界   
        long now = millis();//记录当前时间
        if (now - lastMsg > 500) {//每隔500毫秒秒发一次数据（当小于250毫秒时卡进程）
           encodeJson();
           client.publish("ImCar",msg);//以ImCar为TOPIC对外发送MQTT数据
          lastMsg = now;//刷新上一次发送数据的时间
        }
       }
}

void seek_wheel(){//通过原地旋转寻找两电机在物理上的特性
     long now_setup = millis();
     if(now_setup-last[1]>500){
      op++;
      last[1]=millis();
     }
     if(op==2){
      execute(0,1,1023,1023);
     }
     if(op==3){
        pulse_num();
     }
     if(op==4){ 
      execute(1,0,1023,1023);
        if(count_dif[0]>0){
            wheel=0;//定左轮
           }else{
            wheel=1;//定右轮
         }
     }
     if(op==6){
        execute(0,1,0,0);
     }
}

void ept_values(int ept){//初始化变量
  if(ept==1){//全部初始化（除execute_straight,command_s）
    L_speed[0]=0;
    R_speed[0]=0;
    L_speed[1]=0;
    R_speed[1]=0;
    L_speed[2]=0;
    R_speed[2]=0;
    count_dif[0]=0;
    count_dif[1]=0;
    adjust_count=0;
    PWM_temp[1]=0;
    PWM_temp[0]=0;
    PWM[0]=0;
    PWM[1]=0;
    DT=50;
    Dcount[0]=0;
    Dcount[1]=0;
  }
  else if(ept==2){//测速时间变化时使用
     L_speed[0]=0;
    R_speed[0]=0;
    L_speed[1]=0;
    R_speed[1]=0;
    L_speed[2]=0;
    R_speed[2]=0;
    count_dif[0]=0;
    count_dif[1]=0;
    PWM_temp[1]=0;
    PWM_temp[0]=0;
    command_s=0;
  }
  else{
    //do nothing
  }
}



void straight(int dir){//走直算法，其中dir=1表向前走，dir=0代表向后退
  if(adjust_count>=3)//第四次开始后的所有调整
  {
      if(count_dif[1]<0&&Dcount[0]>Dcount[1])
      {
          if(wheel==0)//定左轮
          {
           PWM[1]= PWM[1]+abs(count_dif[1])*PID[1];
              if(PWM[1]>1023)
              {
               PWM[1]=1023;
               }
          }
          else
          {
           PWM[0]=PWM[0]-abs(count_dif[1])*PID[1];
          }
       Dcount[1]=Dcount[0]+1;
      }
     else if(count_dif[1]>0&&Dcount[0]>Dcount[1])
     {
         if(wheel==1)//定右轮
         {
          PWM[0]= PWM[0]+abs(count_dif[1])*PID[1];
             if(PWM[0]>1023)
             {
              PWM[0]=1023;
              }
          }
             else
             {
              PWM[1]= PWM[1]-abs(count_dif[1])*PID[1];
             }
      Dcount[1]=Dcount[0]+1;
      }
  execute(dir,dir,PWM[0],PWM[1]);
  }

  
  else if(adjust_count<3){//前三次为非稳定期，不用走直算法进行调整。
    if(dir==2){
      //do nothing
    }
    else if(dir==1||dir==0){
      if(wheel==0){//定左轮
        PWM[0]=command_s;
       if(adjust_count==0){
        PWM[1]=command_s;
        adjust_count++;Dcount[1]=Dcount[0];
       }
        if(count_dif[1]>0&&Dcount[0]>Dcount[1]){//右轮快
          adjust_count++;Dcount[1]=Dcount[0]+1;
        }
        else if(count_dif[1]<0&&Dcount[0]>Dcount[1]){//右轮慢
          adjust_count++;Dcount[1]=Dcount[0]+1;
        }
      }
      if(wheel==1){//定右轮
        PWM[1]=command_s;
        if(adjust_count==0){
        adjust_count++;Dcount[1]=Dcount[0];
       }
        if(count_dif[1]>0&&Dcount[0]>Dcount[1]){//左轮慢
          adjust_count++;Dcount[1]=Dcount[0]+1;
        }
        else if(count_dif[1]<0&&Dcount[0]>Dcount[1]){//左轮快
          adjust_count++;Dcount[1]=Dcount[0]+1;
        }
      }
           execute(dir,dir,PWM[0],PWM[1]);
    }
    else{
      //do nothing
    }
  }
}

void pulse_num(){ //测量脉冲
  pulse_listen[0]=digitalRead(D7);
  if(!pulse_change[0]){
     pulse_change[2]= pulse_listen[0];
    pulse_change[0]=1;
  }
  if( pulse_listen[0]!= pulse_change[2]){
    L_speed[0]+=1;
    L_speed[1]+=1;
     pulse_change[2]= pulse_listen[0];
  }
   pulse_listen[1]=digitalRead(D6);
  if(!pulse_change[1]){
     pulse_change[3]= pulse_listen[1];
    pulse_change[1]=1;
  }
  if( pulse_listen[1]!=pulse_change[3]){
    R_speed[0]+=1;
    R_speed[1]+=1;
    pulse_change[3]= pulse_listen[1];
  }
  count_dif[0]=R_speed[0]-L_speed[0];
 detection_count(DT);
}

void detection_count(int Delta_T){   //每 Delta_T毫秒测一次码盘数据，发布版在_speed[2]中
  long speed_detection_time_now=millis();
  long Delta_T_Real=speed_detection_time_now-last[0];
  if(Delta_T_Real>Delta_T){
        R_speed[2]=R_speed[1]*Delta_T/(Delta_T_Real);  
        L_speed[2]=L_speed[1]*Delta_T/(Delta_T_Real);
        R_speed[1]=0; 
        L_speed[1]=0;
        count_dif[1]=R_speed[2]-L_speed[2];
        last[0]=speed_detection_time_now;
        Dcount[0]++;
    }
  }

void encodeJson(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root1 = jsonBuffer.createObject();
  //发送数据区上界
root1["RS"] =  R_speed[2];//右码盘50毫秒速度
root1["S"] =  command_s;//指令要求速度
root1["LS"] =  L_speed[2];//左码盘50毫秒速度
root1["DA"] =  count_dif[0]; //本次调速总偏差
root1["DB"] =  count_dif[1]; //每50毫秒偏差
root1["PL"] =  PWM[0]; //左码盘PWM值
root1["PR"] = PWM[1]; //右码盘PWM值
root1["w"] =  wheel;//为1，车的属性是右轮快，为0，车的属性是左轮快
root1["C"] =  adjust_count;//本次走直调整次数
  //发送数据区下界
  root1.printTo(msg);
  }

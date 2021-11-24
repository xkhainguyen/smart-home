#include <PubSubClient.h>       // MQTT lib
#include <ESP8266WiFi.h>        // wifi lib
#include <ESP8266WiFiMulti.h>   // multiwifi lib
#include <stdio.h>

#define WIFI_SSID "Xuan khai" // Put your WIFI_SSID here
#define PASSWORD "0912821827" // Put your wifi password here
#define TOKEN "BBFF-K6jDr1fjp9cvAjGpQ4g4DxZrqdNtYQ" // Put your Ubidots' TOKEN
#define DEVICE_LABEL "nodeMCU" // Assign the device label to subscribe
#define MQTT_CLIENT_NAME "khai" // MQTT client Name, put a Random ASCII

#define RELAY 15

//char mqtt_server_Local[] = "";
char mqtt_server_Cloud[] = "industrial.api.ubidots.com";

ESP8266WiFiMulti WiFiMulti;
//WiFiClient espClient1;
WiFiClient espClient2;

//PubSubClient client1(espClient1);
PubSubClient client2(espClient2);

void MQTTInit()
{
  WiFiMulti.addAP(WIFI_SSID, PASSWORD);
  delay(200);
  while(WiFiMulti.run()!= WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

//  client1.setServer(mqtt_server_Local, 1883);
  client2.setServer(mqtt_server_Cloud, 1883);
//  client1.setCallback(callback);
  client2.setCallback(callback);
}

void reconnect() 
{
  // Loop until we're reconnected
  if (!client2.connected()) 
  {
    Serial.println("Attempting MQTT connection...");
  
    // Attempt to connect
    if (client2.connect(MQTT_CLIENT_NAME, TOKEN,"")) 
    {
      Serial.println("connected");        
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(client2.state());
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  char value[20];
  
  // Fills array with null values
  for (int i = 0; i < 20; i++) 
  {
    value[i] = '\0';
  }
  
  for (int i=0;i<length;i++) 
  {
    value[i] = payload[i];
  }

  if (strcmp(topic,"/v1.6/devices/nodemcu/light/lv")==0)
  {
    if (value[0] == '1') 
    {
      digitalWrite(RELAY, LOW);
      Serial.println("LIGHT ON");
    }
    else 
    {
      digitalWrite(RELAY, HIGH);
      Serial.println("LIGHT OFF");
    }
  }
} 

void setup() {
  // put your setup code here, to run once:
  pinMode(RELAY, OUTPUT);
  Serial.begin(115200);
  Serial.println("123");
  MQTTInit();
}

void loop() {
  
  // put your main code here, to run repeatedly:
  if (!client2.connected())  //xu li mang trong nay 
  {
    reconnect();
    // Sau khi reconnect thi subscribe cac topic
    char topicToSubscribe[200] ="";
    sprintf(topicToSubscribe, "%s", "/v1.6/devices/nodeMCU/light/lv");
    client2.subscribe(topicToSubscribe);
  }
  client2.loop();
}

/*************************************************
 *    Template for static IP,MQTT &DeepSleep     *
 *      Read out a DHT11                         *  
 *************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
IPAddress ip(192,168,1,50);// pick your own IP outside the DHCP range of your router
IPAddress gateway(192,168,1,1);//watch out, these are comma's not dots
IPAddress subnet(255,255,255,0);
IPAddress dns(192,168,1,1);
//---------voor pubsub
WiFiClient EspClient;
PubSubClient client(EspClient);
//---------Other------
String name = (__FILE__);     // filename
unsigned int batt;
ADC_MODE(ADC_VCC); // vcc uitlezen.
//--------DHT11------
float  humid;
float temp;
#include <DHT.h>
#define DHTTYPE  DHT11
#define DHTPIN 2 // D4
DHT dht(DHTPIN, DHTTYPE, 11); //

//D0 connects RST after boot.

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("WAKED UP AND DOING MY JOB");
  dht.begin();
  //strip name
  //strip path van filename
  byte p1 = name.lastIndexOf('\\');
  byte p2 = name.lastIndexOf('.');
  name = name.substring(p1 + 1, p2);
  batt = ESP.getVcc();
  // Set up WiFi
  WiFi.config(ip,dns,gateway,subnet);
  WiFi.begin("Xuan khai","0912821827");
  //---------
  client.setServer("192.168.1.4",1883);//your MQTT server's IP.Mind you, these are separated by dots again
  //client.setCallback(callback);  
  sendMQTTmessage();
  /* Close WiFi connection */
  EspClient.stop();
  Serial.println("GOING TO SLEEP AND WILL WAKE UP AFTER 30 SECONDS");
  ESP.deepSleep(30*1000000);//1 sec
}

void loop() {

}

void sendMQTTmessage()
{
  if (!client.connected()) {
  reconnect();  }
  client.publish("home/outside/name", name.c_str(),false);
  client.publish("home/outside/stat/batt",String(batt/1000.0).c_str(),false);
  client.publish("home/outside/stat/temp",String(dht.readTemperature()).c_str(),false);
  client.publish("home/outside/stat/humid",String(dht.readHumidity()).c_str(),false);
  Serial.println("Published");
   /* Close MQTT client cleanly */
  client.disconnect();
}

void reconnect(){
  while (!client.connected())
  {
    String ClientId="ESP8266";
    ClientId +=String(random(0xffff),HEX);
    if (client.connect(ClientId.c_str()))
      //if your MQTT server is protected with a password, use the next line instead of the revious
      //if (client.connect(ClientId.c_str()),mqtt_user,mqtt_password))
    {
      Serial.println("Connected");
      client.publish("home/outside/stat/connection","OK");
      }
     else{
      Serial.print("failed, rc= ");
      Serial.println(client.state());
      delay(1000);
      } 
    }
}

    

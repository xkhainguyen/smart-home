//=====================********=======================//
//====================N H O M  9======================//
//================S M A R T  H O M E==================//
//=====================********=======================//

//=================E S P 8 2 6 6 - 2==================//
//============Control Bathroom and Bedroom============//

//===============LIBRARIES===============//
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // LCD lib
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <BH1750.h>             // GY-30
#include <TimeLib.h>            // RTC Time
#include <WidgetRTC.h>

//===============DEFINES===============//
//SOME USED PINS
//I2C connection: D1-SCL, D2-SDA
#define WC_LIGHT_PIN  D7        // D7/GPIO13
#define WATER_PIN     D8        // D8/GPIO15
#define HEAT_PIN      D6        // D6/GPIO12
#define WC_DHT_PIN    D9        // GPIO1

#define BR_LIGHT_PIN  14        // D5/GPIO14
#define OUT_DHT_PIN   D10        // D3/GPIO3/RX

//SOME DEVICES
#define DHT_TYPE DHT11   // DHT 11

//SOME INTERVAL
#define INTERVAL_WC_DHT 1000  // Trich mau DHT11 trong phong tam moi 100 miliseconds
#define INTERVAL_OUT_DHT 3000
#define INTERVAL_BR_GY 1000
//---------------------------------------//
//===============SOME OBJECTS===============//
DHT wc_dht(WC_DHT_PIN, DHT_TYPE);
DHT out_dht(OUT_DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x3F, 16, 2);
BH1750 brGY;
BlynkTimer connectionHandlerTimer;
WidgetRTC rtc;
WidgetBridge bridge1(V50);
//===============SOME EXTRA THINGS===============//
typedef enum {
  CONNECT_TO_WIFI,
  AWAIT_WIFI_CONNECTION,
  CONNECT_TO_BLYNK,
  AWAIT_BLYNK_CONNECTION,
  MAINTAIN_CONNECTIONS,
  AWAIT_DISCONNECT
} CONNECTION_STATE;

CONNECTION_STATE connectionState;
uint8_t connectionCounter;
//---------------------------------------//
//===============SOME VARIABLES===============//
//TIME VARIABLES
String currentTime;
String currentDate;

unsigned long currentRun = 0;
unsigned long prevRunWCDHT = 0;
unsigned long prevRunOutDHT = 0;
unsigned long prevRunBRGY = 0;
//SOME DATA
char auth[] = "IcM4rsshNqEIahAriMl2H_NufuCDHNLj";
char ssid[] = "Khai";
char pass[] = "00000000";

float wcTemp = 0, wcHum = 0;
float outTemp = 0, outHum = 0;
float brLux = 0;
//CONTROL VARIABLES
float wcTempE = 0, wcTempUk = 1023, wcTempUk1 = 0;
float wcTempSP = 0;
bool wcTempControlEN = false;
float brLuxE = 0, brLuxUk = 500, brLuxUk1 = 0;
float brLuxSP = 200;
bool brLuxControlEN = false, brNightModeEN = false;

bool enterState = false, exitState = false;
bool eSavingModeEN = true;
//---------------------------------------//
//===============CONFIGURE EACH DEVICE===============//
void configGPIO()
{
  pinMode(WC_LIGHT_PIN, OUTPUT);
  digitalWrite(WC_LIGHT_PIN,0);
  pinMode(WATER_PIN, OUTPUT);
  digitalWrite(WATER_PIN,0);
  pinMode(HEAT_PIN, OUTPUT);
  analogWrite(HEAT_PIN,0);

  pinMode(BR_LIGHT_PIN, OUTPUT);
  analogWrite(BR_LIGHT_PIN, 0);
}
void configWIFI()
{
  connectionHandlerTimer.setInterval(100, ConnectionHandler);
  connectionState = CONNECT_TO_WIFI;
}
void configWC()
{
  wc_dht.begin();
}
void configOut()
{
  out_dht.begin();
}
void configBR()
{
  brGY.begin();
}
void configLCD()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
}
void configRTC()
{
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
}
//---------------------------------------------------//
//===============SOME FUNCTIONS===============//
void ConnectionHandler(void) {
  switch (connectionState) {
  case CONNECT_TO_WIFI:
    BLYNK_LOG("Connecting to %s.", ssid);
    WiFi.begin(ssid, pass);
    connectionState = AWAIT_WIFI_CONNECTION;
    connectionCounter = 0;
    break;

  case AWAIT_WIFI_CONNECTION:
    if (WiFi.status() == WL_CONNECTED) {
      BLYNK_LOG("Connected to %s", ssid);
      connectionState = CONNECT_TO_BLYNK;
    }
    else if (++connectionCounter == 50) {
      BLYNK_LOG("Unable to connect to %s. Retry connection.", ssid);
      WiFi.disconnect();
      connectionState = AWAIT_DISCONNECT;
      connectionCounter = 0;
    }
    break;

  case CONNECT_TO_BLYNK:
    BLYNK_LOG("Attempt to connect to Blynk server.");
    Blynk.config(auth, IPAddress(149,28,129,219), 8080);
    Blynk.connect();
    connectionState = AWAIT_BLYNK_CONNECTION;
    connectionCounter = 0;
    break;

  case AWAIT_BLYNK_CONNECTION:
    if (Blynk.connected()) {
      BLYNK_LOG("Connected to Blynk server.");
      connectionState = MAINTAIN_CONNECTIONS;
    }
    else if (++connectionCounter == 50) {
      BLYNK_LOG("Unable to connect to Blynk server. Retry connection.");
      Blynk.disconnect();
      WiFi.disconnect();
      connectionState = AWAIT_DISCONNECT;
      connectionCounter = 0;
    }
    break;

  case MAINTAIN_CONNECTIONS:
    if (WiFi.status() != WL_CONNECTED) {
      BLYNK_LOG("Wifi connection lost. Reconnect.");
      Blynk.disconnect();
      WiFi.disconnect();
      connectionState = AWAIT_DISCONNECT;
      connectionCounter = 0;
    }
    else  if (!Blynk.connected()) {
      BLYNK_LOG("Blynk server connection lost. Reconnect.");
      Blynk.disconnect();
      connectionState = CONNECT_TO_BLYNK;
    }
    else {
      Blynk.run();
    }
    break;

  case AWAIT_DISCONNECT:
    if (++connectionCounter == 10) {
      connectionState = CONNECT_TO_WIFI;
    }
    break;
  }
}

void processWCDHT()
{
  if (currentRun - prevRunWCDHT >= INTERVAL_WC_DHT) 
  {
    prevRunWCDHT = currentRun; //store led1 state change time
    wcHum = wc_dht.readHumidity();
    wcTemp = wc_dht.readTemperature();
    if (isnan(wcTemp)) 
    {
      Serial1.println(F("Failed to read from WC DHT sensor!"));
    }
    Blynk.virtualWrite(V23, wcTemp);
    Blynk.virtualWrite(V24, wcHum);
//    Serial1.print(wcTemp);
//    Serial1.print(" ");
    
    if(wcTempControlEN) 
    {
      wcTempController(); //dieu khien 
    }
    else 
    {
      wcTempUk = 1023;
      analogWrite(HEAT_PIN, 0);
    }
  }  
}

void processOutDHT()
{
  if (currentRun - prevRunOutDHT >= INTERVAL_OUT_DHT) 
  {
    prevRunOutDHT = currentRun; //store led1 state change time
    outHum = out_dht.readHumidity();
    outTemp = out_dht.readTemperature();
    if (isnan(outTemp)) 
    {
      Serial1.println(F("Failed to read from OUT DHT sensor!"));
    }
    Blynk.virtualWrite(V41, outTemp); 
    Blynk.virtualWrite(V42, outHum); 
    bridge1.virtualWrite(V51, outTemp);
    bridge1.virtualWrite(V52, outHum);
  }
}

void wcTempController()
{
  wcTempE = wcTempSP - wcTemp;        //sai lech
  if(isnan(wcTempE)) 
  {
    wcTempUk = 0;   //trach doc loi dan toi loi dieu khien
    wcTempE = 0;
  }
  wcTempUk1 = wcTempUk + 100*wcTempE; //bo dieu khien PI voi P=0 Uk1 la Uk+1
  wcTempUk1 = constrain(wcTempUk1, 0, 1023); //gioi han tin hieu dieu khien trong kha nang
  wcTempUk = wcTempUk1;
//  Serial1.println(wcTempUk); 
//  Serial1.println(wcTempSP); 
  analogWrite(HEAT_PIN, wcTempUk);  ; //xuat xung PWM
}
void processBRGY()
{
  if (currentRun - prevRunBRGY >= INTERVAL_BR_GY) 
  {
    prevRunBRGY = currentRun; //store led1 state change time
    brLux = brGY.readLightLevel();
    if (brLux<0) 
    {
      Serial1.println(F("Failed to read from BR GY30 sensor!"));
    }
    Blynk.virtualWrite(V31, brLux);
    if(brLuxControlEN) 
    {
      brLuxController(); //dieu khien anh sang
    }
    else 
    {
      brLuxUk = 500;
      analogWrite(BR_LIGHT_PIN, 0);
    }
  }  
}

void brLuxController()
{
  brLuxE = brLuxSP - brLux;        //sai lech
  if(brNightModeEN)
  {
    brLuxUk1 = (24 - hour())*50;
  }
  else brLuxUk1 = brLuxUk + 0.5*brLuxE; //bo dieu khien PI voi P=0 Uk1 la Uk+1
  brLuxUk1 = constrain(brLuxUk1, 0, 1023); //gioi han tin hieu dieu khien trong kha nang
  brLuxUk = brLuxUk1;
//  Serial1.println(brLuxUk); 
//  Serial1.println(brLux); 
  analogWrite(BR_LIGHT_PIN, brLuxUk);  ; //xuat xung PWM
}

void processESavingMode2()
{
  digitalWrite(WC_LIGHT_PIN,0);
  Blynk.virtualWrite(V20,0);
  brLuxControlEN = false;
  Blynk.virtualWrite(V30,0); 
}
//SOME BLYNK FUNCTIONS
BLYNK_CONNECTED() {
  // Synchronize time on connection
  Blynk.syncAll();
  rtc.begin();
  bridge1.setAuthToken("WjrMI098MjgkRzZ0neqy-Oo8INW7mRka");
  
}
//for wc (bathroom)
BLYNK_WRITE(V20)
{
  int pinValue = param.asInt(); 
  Serial1.println("BATHROOM LIGHT");
  digitalWrite(WC_LIGHT_PIN, pinValue);
}
BLYNK_WRITE(V21)
{
  int pinValue = param.asInt(); 
  digitalWrite(WATER_PIN, pinValue);
  Serial1.println("BATHROOM WATER HEATER");
}
BLYNK_WRITE(V22)
{
  int pinValue = param.asInt();
  Serial1.println("BATHROOM TEMPERATURE CONTROL");

    wcTempControlEN = pinValue;
    wcTempSP = pinValue;
}
BLYNK_WRITE(V30)
{
  int pinValue = param.asInt(); 
  brLuxControlEN = pinValue;
  Serial1.println("BEDROOM LIGHT");
}
BLYNK_WRITE(V32)
{
  int pinValue = param.asInt(); 
  Serial1.println("BEDROOM NIGHT MODE");
  brNightModeEN = pinValue;
}
BLYNK_WRITE(V33)  // trigger for ESP8266-1
{
  int pinValue = param.asInt(); 
  Serial1.println("WAKEUP TIME TRIGGER");
  bridge1.virtualWrite(V11,1);  // make coffee
}
BLYNK_WRITE(V45)
{
  int pinValue = param.asInt();
  enterState = pinValue;
  Serial1.println("GPS ENTER");
}
BLYNK_WRITE(V46)
{
  Serial1.println(exitState);
  int pinValue = param.asInt();
  Serial1.println(pinValue);
  if(eSavingModeEN&&(!exitState)&&pinValue){
    processESavingMode2();
    Serial1.println("ALL LIGHT OFF");
    bridge1.virtualWrite(V53, 1);
  }
  exitState = pinValue;
  Serial1.println("GPS EXIT");
}
BLYNK_WRITE(V53)
{
  int pinValue = param.asInt();
  eSavingModeEN = pinValue;
  Serial1.println("ENERGY SAVING MODE");
}
//---------------------------------------------------//
//===============MAIN PROGRAM===============//
void setup() {
  Serial1.begin(9600);
  configGPIO();
  configWIFI();
  configWC();
  configOut();
  configLCD();
  configBR();
  configRTC();
}

void loop() {
  currentRun = millis();
  processWCDHT();
  processOutDHT();
  processBRGY();
  connectionHandlerTimer.run();
}

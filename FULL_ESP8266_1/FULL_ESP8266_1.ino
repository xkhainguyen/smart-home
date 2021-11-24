//=====================********=======================//
//====================N H O M  9======================//
//================S M A R T  H O M E==================//
//=====================********=======================//

//=================E S P 8 2 6 6 - 1==================//
//===========Control Living Room and Kitchen==========//

//===============LIBRARIES===============//
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>  // LCD lib
#include <WidgetRTC.h>
#include <TimeLib.h>            // RTC Time

//---------------------------------------//
//===============DEFINES===============//
//SOME USED PINS
//I2C connection: D1-SCL, D2-SDA
#define LR_LIGHT_PIN  4       // Living Room Light at 4 of PCA
#define RGB_R_PIN     1       // of PCA
#define RGB_G_PIN     2       // of PCA
#define RGB_B_PIN     3       // of PCA
#define BUZZER_PIN    D8       // GPIO13

#define KC_LIGHT_PIN  15       // Kitchen Light at 15 of PCA
#define CF_MAKER_PIN  14       // Coffee maker at 14 of PCA
#define FIRE_PIN      D3      // GPIO0

#define OUT_LIGHT_PIN 0       // of PCA
#define LOCK_PIN      5      // Red LED for door lock
#define PIR_PIN       D6      // GPIO9
#define RFID_PIN      D7      // GPIO10
#define CAMERA_PIN    D5      // GPIO3
//SOME DEVICES

//SOME INTERVAL
#define INTERVAL_FIRE 1000
#define INTERVAL_PIR  1000
#define INTERVAL_BUZZER 1000
#define INTERVAL_DOOR_LOCK 500
#define INTERVAL_PASSWORD 20000
#define INTERVAL_COFFEE 10000
//---------------------------------------//
//===============SOME OBJECTS===============//
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
LiquidCrystal_I2C lcd(0x3F, 16, 2);
BlynkTimer connectionHandlerTimer;
WidgetRTC rtc;
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
unsigned long prevRunFire = 0;
unsigned long prevRunPIR = 0;
unsigned long prevRunBuzzer = 0;
unsigned long prevRunDoorLock = 0;
unsigned long prevRunPassword = -INTERVAL_PASSWORD;
unsigned long prevRunCoffee = 0;

//SOME DATA
char auth[] = "WjrMI098MjgkRzZ0neqy-Oo8INW7mRka";
char ssid[] = "Khai";
char pass[] = "00000000";

bool rfidState = false, cameraState = false;
bool fireState = false, firePreState = false;
bool openState = false;
bool buzzerState = false;
bool pirState = false, pirPreState = false;
bool securityModeEN = false;
bool lrAutoLightEN = false;
bool password = false, cfState = false;
bool enterState = false, exitState = false;
bool wkTimeTrig = false;

float outTemp = 0, outHum = 0;
//---------------------------------------//
//===============CONFIGURE EACH DEVICE===============//
void configGPIO()
{
  pinMode(FIRE_PIN, INPUT_PULLUP);
  pinMode(CAMERA_PIN, INPUT_PULLUP);
  pinMode(RFID_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); 
  digitalWrite(BUZZER_PIN, LOW);
}
void configWIFI()
{
  connectionHandlerTimer.setInterval(100, ConnectionHandler);
  connectionState = CONNECT_TO_WIFI;
}
void configPCA()
{
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(1000);  // This is the maximum PWM frequency
}
void configLR()
{
  pwm.setPWM(RGB_R_PIN, 0, 4095);
  pwm.setPWM(RGB_G_PIN, 0, 4095);
  pwm.setPWM(RGB_B_PIN, 0, 4095);
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
      Blynk.config(auth, IPAddress(149, 28, 129, 219), 8080);
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

void processFire()  //xu li cam bien lua
{
  if (currentRun - prevRunFire >= INTERVAL_FIRE)
  {
    prevRunFire = currentRun;
    firePreState = fireState;
    fireState = !digitalRead(FIRE_PIN);  //Co lua thi D0 la 0, LED D0 sang, dao trang thai
    //    Serial.println("fireState:"+String(fireState));
    //    Serial.println("firePreState:"+String(firePreState));
  }
}

void processPIR()  //xu li PIR
{
  if (currentRun - prevRunPIR >= INTERVAL_PIR)
  {
    prevRunPIR = currentRun;
    pirPreState = pirState;
    pirState = digitalRead(PIR_PIN);  // co motion thi 1, khong thi 0
    //    Serial.println("pirState:"+String(pirState));
    //    Serial.println("pirPreState:"+String(pirPreState));
  }
}
void processDoorLock()
{
  if (currentRun - prevRunDoorLock >= INTERVAL_DOOR_LOCK)
  {
    prevRunDoorLock = currentRun;
    rfidState = digitalRead(RFID_PIN);
    if(enterState) rfidState = true;  //trong khu vuc nha thi bo qua rfid
    cameraState = digitalRead(CAMERA_PIN);
    if (rfidState&&cameraState)
    {
      openState = 1;
      Blynk.virtualWrite(V5, 1);
    }
  }
  pwm.setPWM(LOCK_PIN, 0, (!openState)*4095);
}
void processPassword()  // pass co hieu luc trong vong 10s
{
  if(currentRun - prevRunPassword >= INTERVAL_PASSWORD) {
    if(password){
      password = false;
      Blynk.virtualWrite(V4, password*255);
    }
  }
  
//  Serial.println("password:" + String(password));
}
void processCoffee()  // thoi gian pha coffee la 2s
{
  if(currentRun - prevRunCoffee >= INTERVAL_COFFEE) { 
    if(cfState) {
      Blynk.notify("Coffee is done. Enjoy it!");
      cfState = false;
      Blynk.virtualWrite(V11, cfState);
    }
   }
   pwm.setPWM(CF_MAKER_PIN, 0, cfState * 4095);
}
void processBuzzer()  //xu li coi
{
  if (currentRun - prevRunBuzzer >= INTERVAL_BUZZER)
  {
    prevRunBuzzer = currentRun;
    if ((firePreState == 0) && (fireState == 1)) {
      buzzerState = 1;
      Blynk.notify("The Fire Alarm has been activated at Your House");
    }
    if ((pirPreState == 0) && (pirState == 1)) {
      if (securityModeEN) {
        buzzerState = 1;
        Blynk.notify("The Motion Alarm has been activated at Your House");
      }
      if (lrAutoLightEN) {
        pwm.setPWM(LR_LIGHT_PIN, 0, 4095);
        Blynk.virtualWrite(V0, 1);
      }
    }

    //    if((firePreState==1)&&(fireState==0)){
    //      buzzerState = 0;
    //      Blynk.notify("The Fire Alarm has been deactivated at Your House");
    //    }
    //    if((pirPreState==1)&&(pirState==0)){
    //      buzzerState = 0;
    //      Blynk.notify("The Motion Alarm has been deactivated at Your House");
    //    }
//    Serial.println("buzzerState:" + String(buzzerState));
    Blynk.virtualWrite(V1, buzzerState);
    digitalWrite(BUZZER_PIN, buzzerState);
  }
}

void displayLCD()
{
//  In ra LCD nhiet do va do am ben ngoai
  lcd.setCursor(0,0);
  lcd.print("To:");
  lcd.print(round(outTemp),0);
  lcd.print("\337C");
  lcd.setCursor(8,0);
  lcd.print("Ho:");
  lcd.print(round(outHum),0);
  lcd.print("%");
}
void processESavingMode1()
{
  pwm.setPWM(LR_LIGHT_PIN, 0, 0);
  Blynk.virtualWrite(V0,0);
  pwm.setPWM(KC_LIGHT_PIN, 0, 0);
  Blynk.virtualWrite(V10 ,0);
  pwm.setPWM(OUT_LIGHT_PIN, 0, 0);
  Blynk.virtualWrite(V40,0);
}
//SOME BLYNK FUNCTIONS
BLYNK_CONNECTED() {
  // Synchronize time on connection
  Blynk.syncAll();
  rtc.begin();
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt();
  pwm.setPWM(LR_LIGHT_PIN, 0, 4095 * pinValue);
  Serial.println("LIVING ROOM LIGHT");
}
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  if (password) buzzerState = pinValue;
  Serial.println("LIVING ROOM BUZZER");
}
BLYNK_WRITE(V2) //RGB chung Vcc la chan dai nhat. R V G B
{
  int r = param[0].asInt();
  int g = param[1].asInt();
  int b = param[2].asInt();
  pwm.setPWM(RGB_R_PIN, 0, 4095 - r);
  pwm.setPWM(RGB_G_PIN, 0, 4095 - g);
  pwm.setPWM(RGB_B_PIN, 0, 4095 - b);
}
BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
  lrAutoLightEN = pinValue;
  Serial.println("LIVING ROOM AUTO LIGHT MODE");
}
BLYNK_WRITE(V4) // Password
{
  int pinValue = param.asInt();
  password = pinValue;
  if(password) prevRunPassword = currentRun;
//  Serial.println("pinValue:"+String(pinValue));
  Serial.println("PASSWORD");
}
BLYNK_WRITE(V5) // Khoa cua
{
  int pinValue = param.asInt();
  if(password) openState = pinValue;
  else {  
    if(pinValue == 0) openState = pinValue;
    if(pinValue == 1) Blynk.virtualWrite(V5, 0);
  }
  Serial.println("DOOR LOCK");
}
BLYNK_WRITE(V10)
{
  int pinValue = param.asInt();
  pwm.setPWM(KC_LIGHT_PIN, 0, 4095 * pinValue);
  Serial.println("KITCHEN LIGHT");
}
BLYNK_WRITE(V11)
{
  int pinValue = param.asInt();
  cfState = pinValue;
  if(cfState) prevRunCoffee = currentRun;
  Serial.println("COFFEE MAKER");
}
//BLYNK_WRITE(V33)  //
//{
//  int pinValue = param.asInt();
//  wkTimeTrig = pinValue;
//  Serial.println("WAKEUP TIME TRIGGER");
//}
BLYNK_WRITE(V40)
{
  int pinValue = param.asInt();
  pwm.setPWM(OUT_LIGHT_PIN, 0, 4095 * pinValue);
  Serial.println("OUTSIDE LIGHT");
}
BLYNK_WRITE(V43)
{
  int pinValue = param.asInt();
  securityModeEN = pinValue;
  Serial.println("SECURITY MODE");
}
BLYNK_WRITE(V45)
{ 
  Serial.println(enterState);
  int pinValue = param.asInt();
  Serial.println(pinValue);
  enterState = pinValue;
  Serial.println("GPS ENTER");
}
BLYNK_WRITE(V46)
{
  int pinValue = param.asInt();
  exitState = pinValue;
  Serial.println("GPS EXIT");
}
BLYNK_WRITE(V51)
{
  int pinValue = param.asInt();
  outTemp = pinValue;
//  Serial.println("OUTSIDE TEMPERATURE");
}
BLYNK_WRITE(V52)
{
  int pinValue = param.asInt();
  outHum = pinValue;
//  Serial.println("OUTSIDE HUMIDITY");
}
BLYNK_WRITE(V53)
{
  int pinValue = param.asInt();
  if(pinValue==1) processESavingMode1();
  Serial.println("ALL LIGHT OFF");
}
//---------------------------------------------------//
//===============MAIN PROGRAM===============//
void setup() {
  Serial.begin(9600);
  configGPIO();
  configPCA();
  configLCD();
  configWIFI();
  configLR();
  configRTC();
}

void loop() {
  currentRun = millis();
  displayLCD();
  processFire();
  processPIR();
  processBuzzer();
  processPassword();
  processDoorLock();
  processCoffee();
  connectionHandlerTimer.run();
}

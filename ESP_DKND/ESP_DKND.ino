#include "DHT.h"

#define DHT_PIN D9    // Digital pin connected to the DHT sensor
#define HEAT_PIN D6
#define DHT_TYPE DHT11   // DHT 11
#define INTERVAL_DHT11 1000  // Trich mau BMP11 moi 100 miliseconds
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long currentTime = 0;
unsigned long prevTimeDHT11 = 0;

float wcTemp = 0, wcTempE = 0, wcTempUk = 0, wcTempUk1 = 0;
float wcTempSP = 27.0;
bool wcTempControlEN = false;

void processDHT11()
{
  if (currentTime - prevTimeDHT11 >= INTERVAL_DHT11) 
  {
    prevTimeDHT11 = currentTime; //store led1 state change time
    //  h = dht.readHumidity();
    wcTemp = dht.readTemperature();
    if (isnan(wcTemp)) 
    {
      Serial.println(F("Failed to read from DHT sensor!"));
    }
    Serial.print(wcTemp);
    Serial.print(" ");
    
    if(wcTempControlEN) 
    {
      tempController(); //dieu khien 
    }
    else 
    {
      wcTempUk = 0;
      analogWrite(HEAT_PIN, 0);
    }
  }  
}

void tempController()
{
  wcTempE = wcTempSP - wcTemp;        //sai lech
  wcTempUk1 = wcTempUk + 10*wcTempE; //bo dieu khien PI voi P=0
    Serial.println(wcTempUk1); 
  wcTempUk1 = constrain(wcTempUk1, 0, 1023); //gioi han tin hieu dieu khien trong kha nang
  wcTempUk = wcTempUk1;
  Serial.println(wcTempE);

  analogWrite(HEAT_PIN, wcTempUk); //xuat xung PWM
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));
  pinMode(HEAT_PIN, OUTPUT);
  analogWrite(HEAT_PIN, 0);
  dht.begin();
  wcTempControlEN = true;
}

void loop() 
{
  currentTime = millis(); //read and store current time
  processDHT11();
 }

/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  PWM test - this will drive 16 PWMs in a 'wave'

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These drivers use I2C to communicate, 2 pins are required to  
  interface.

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <BH1750.h>

#define BR_LIGHT_PIN 2       // Bedroom Light at 2 of PCA
#define LR_LIGHT_PIN 0       // Living Room Light at 0 of PCA
#define KC_LIGHT_PIN 1       // Kitchen Light at 1 of PCA
#define WC_LIGHT_PIN 3       // Bathroom Light at 3 of PCA

#define INTERVAL_GY30 1000  // Trich mau BMP11 moi 100 miliseconds

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
BH1750 lightMeter;

unsigned long currentTime = 0;
unsigned long prevTimeGY30 = 0;

float lux = 0, e = 0, uk = 0, uk1 = 0, duty = 0;
float SP = 1000;
bool brLightControlEN = false;

void processGY30()
{
  lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");  
}

void lightController()
{
  e = SP - lux;        //sai lech
  uk1 = uk + 0.05*e; //bo dieu khien PI voi P=0
  uk1 = constrain(uk1, 0, 4095); //gioi han tin hieu dieu khien trong kha nang
  uk = uk1;
  Serial.println(uk); 
  pwm.setPWM(BR_LIGHT_PIN, 0, uk); //xuat xung PWM
}

void setup() 
{
  Serial.begin(9600);

  pwm.begin();

  lightMeter.begin();
  Serial.println(F("BH1750 Test begin"));

  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(1000);  // This is the maximum PWM frequency 

  brLightControlEN = true;
}

void loop() 
{
  currentTime = millis(); //read and store current time
  if (currentTime - prevTimeGY30 >= INTERVAL_GY30) 
  {
    prevTimeGY30 = currentTime; //store led1 state change time
    processGY30();  //xu li tin hieu cam bien ap suat
    if(brLightControlEN) 
    {
      lightController(); //dieu khien quat
    }
    else 
    {
      uk = 0;
      pwm.setPWM(BR_LIGHT_PIN, 0, 0); 
    }
  }  

}

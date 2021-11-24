#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define LED_INTERVAL 100

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "Lk0cG-4Ui-h7lFnLOghltLO2NTpFdti_";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Xuan khai";
char pass[] = "0912821827";
unsigned long currentTime = 0, prevTimeLED = 0;

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass, IPAddress(149,28,129,219), 8080); //server cua Nhat Anh
}

void loop()
{
  currentTime = millis();
  if((currentTime - prevTimeLED) > LED_INTERVAL)
  {
    prevTimeLED = currentTime;
    Blynk.virtualWrite(V0, digitalRead(0));
  }
  Blynk.run();
}

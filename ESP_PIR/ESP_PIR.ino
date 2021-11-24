/* HC-SR501 Motion Detector */
#define PIR_LED_PIN D5
#define PIR_PIN D7 // Input for HC-S501
bool pirStatus = false; // Place to store read PIR Value
bool securityModeEN = false;

void processPIR(void)
{
  pirStatus = digitalRead(PIR_PIN);
  if (pirStatus)
  {
    Serial.println("==> co trom anh oi"); //feed vao buzzerStatus
  }
  else Serial.println("KHONG");
}

void setup()
{
  Serial.begin(9600);
  delay(10);

  pinMode(PIR_LED_PIN, OUTPUT);
  digitalWrite(PIR_LED_PIN, LOW);

  pinMode(PIR_PIN, INPUT);

}

void loop()
{
  if(securityModeEN)
  {
  processPIR();
  }
  digitalWrite(PIR_LED_PIN, pirStatus);
}

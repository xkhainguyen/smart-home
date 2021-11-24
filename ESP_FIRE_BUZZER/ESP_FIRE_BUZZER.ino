#define FIRE_PIN 13
#define BUZZER_PIN 0

#define INTERVAL_FIRE 1000  

bool fireState = false, buzzerState = false;
bool pirState = false;

unsigned long currentTime = 0;
unsigned long prevTimeFire = 0;

void processFire()  //xu li cam bien lua
{
  fireState = !digitalRead(FIRE_PIN);  //Co lua thi D0 la 0, LED D0 sang, dao trang thai
  Serial.print(fireState);
}

void processBuzzer()  //xu li coi
{
  buzzerState = fireState||pirState; //doc cac loai canh bao
  digitalWrite(BUZZER_PIN, buzzerState);
  Serial.println(buzzerState);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  
  pinMode(FIRE_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

}

void loop() {
  // put your main code here, to run repeatedly:
  currentTime = millis(); //read and store current time
  if (currentTime - prevTimeFire >= INTERVAL_FIRE) 
  {
    prevTimeFire = currentTime; //store led1 state change time
    processFire();  //xu li tin hieu cam bien ap suat
    processBuzzer();
  }
}

//Reader luon duoc bat, quet the neu dung thi den RED nhap nhay, tien hanh doc CAMERA
#include <SPI.h>
#include <MFRC522.h>

//ARD PIN
//SPI SS  10
//SPI MOSI  11
//SPI MISO  12
//SPI SCK 13
//RST 14 means A0

//ESP PIN
//SPI SS  D8
//SPI MOSI  D7
//SPI MISO  D6
//SPI SCK D5

#define RST_PIN         14         // Configurable, see typical pin layout above
#define SS_PIN          10        // Configurable, see typical pin layout above
#define LED_PIN         15 
#define CAMERA_PIN      16

#define INTERVAL_RFID   100
#define INTERVAL_CAMERA 100
#define INTERVAL_DOOR_OPEN 500  //thoi gian mo cua
#define MY_CARD2 "8514125542"   // card phu, chi mo cua
#define MY_CARD1 "172992935"    // card chu, la password
 
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

char uid[13]; //luu ID the 
String str;
bool rfidStatus = false, doorStatus = false;
bool camState = false;

unsigned long currentRun = 0;
unsigned long prevRunRFID = 0;
unsigned long prevRunCamera = 0;
unsigned long prevRunDoorOpen = -INTERVAL_DOOR_OPEN;

void processRFID()  //xu li RFID
{
  if(mfrc522.PICC_IsNewCardPresent())
  {
    mfrc522.PICC_ReadCardSerial();
    sprintf(uid, "%d%d%d%d",mfrc522.uid.uidByte[0],mfrc522.uid.uidByte[1],mfrc522.uid.uidByte[2],mfrc522.uid.uidByte[3]);
    Serial.println(uid);
  }
  else sprintf(uid, "");
  rfidStatus = (!strcmp(uid, MY_CARD1))||(!strcmp(uid, MY_CARD2));
}

void processDoor()
{
  doorStatus = rfidStatus;  //co the dieu khien servo mo cua  //doorStatus = rfidStatus || controlCenter.openDoor
  digitalWrite(LED_PIN, doorStatus);
}
 
void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(CAMERA_PIN, OUTPUT);
  digitalWrite(CAMERA_PIN, LOW);
  Serial.println("Scan PICC to see UID, SAK, type, and data blocks...");
}
 
void loop() {
  currentRun = millis(); //read and store current time
  if (currentRun - prevRunRFID >= INTERVAL_RFID) 
  {
    prevRunRFID = currentRun; 
    processRFID();  //xu li tin hieu
    if (currentRun - prevRunDoorOpen >= INTERVAL_DOOR_OPEN) 
    { 
      processDoor();
      if (doorStatus) prevRunDoorOpen = currentRun;
    }
  }
  if (currentRun - prevRunCamera >= INTERVAL_CAMERA) 
  {
    prevRunCamera = currentRun; 
    str = Serial.readStringUntil('\n');
    if(!strcmp(str.c_str(),"ON")) camState = true;
    else camState = false;
    digitalWrite(CAMERA_PIN, camState);
  }
}

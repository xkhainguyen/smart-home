int pos = 0;    // biến pos dùng để lưu tọa độ các Servo
 
void setup() 
{ 
 pinMode(9, OUTPUT);
} 
 
 
void loop() 
{                            // mỗi bước của vòng lặp tăng 1 độ
    digitalWrite(9, 500);;              // xuất tọa độ ra cho servo
    delay(15);                       // đợi 15 ms cho servo quay đến góc đó rồi tới bước tiếp theo
} 

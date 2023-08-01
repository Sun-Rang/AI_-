#include <Wire.h>

#define address_01 0x04 // 제어 아두이노 I2C 주소

float temperature = 0;
float humidity = 0;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  Wire.begin(address_01);
  Wire.onRequest(handler); // 마스터에게 데이터 송신
  Wire.onReceive(receiveSensor); // 마스터에 의해 데이터 수신시 이벤트 동작
}

void loop() {
  // put your main code here, to run repeatedly:
  //handler();
  delay(100);
}

void handler (){
  Wire.write(10);
}


// 데이터 수신 시 호출되는 이벤트 핸들러
void receiveSensor(int byteCount) {
  if (byteCount == sizeof(float) * 2) {
    Wire.readBytes((byte*)&temperature, sizeof(float)); // 온도 데이터 읽기
    Wire.readBytes((byte*)&humidity, sizeof(float));    // 습도 데이터 읽기

    // 받은 데이터를 이용하여 슬레이브에서 실행할 동작을 수행합니다.
    // 이 예제에서는 받은 데이터를 시리얼 모니터에 출력하는 것으로 대체합니다.
    Serial.print("받은 온도: ");
    Serial.print(temperature);
    Serial.print("도, 습도: ");
    Serial.print(humidity);
    Serial.println("%");
  }
}
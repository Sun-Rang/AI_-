#include "DHT.h"
#include <Wire.h>

// 제어 아두이노 주소 정의
#define address_01 0x04
#define address_02 0x03

// 데이터를 전송하는 함수
// 코드 길이 줄이기 위해
void sendFloatData(byte address, float data) {
  byte* byteArray = (byte*)&data;
  for (int i = 0; i < sizeof(float); i++) {
    Wire.write(byteArray[i]);
  }
}

// 신호센서를 2번핀으로 받음
#define DHTPIN 2
// DHT11제품 확인
#define DHTTYPE DHT11  // DHT 11

// 신호센서를 2번으로 DHT11 제품 사용
DHT dht(DHTPIN, DHTTYPE);

// 나눈 이유는 정수형태로 변환 할 수도 있어서?

float Temperature_read() {
  // 실수형으로 온도센서 읽음
  float t = dht.readTemperature();

  return t;
}

float Humidity_read() {
  // 실수형으로 습도센서 읽음
  float h = dht.readHumidity();

  return h;
}

// WriteData(address_01,Temperature_read(),Humidity_read()); -- (주소값,온도,습도)
void WriteData(byte address, float Temp, float Hum) {

  // 슬레이브 주소를 매개변수로 입력한 주소 값으로 I2C 통신
  Wire.beginTransmission(address);

  sendFloatData(address, Temp);  // 온도 데이터를 슬레이브로 전송
  sendFloatData(address, Hum);   // 습도 데이터를 슬레이브로 전송

  // I2C 통신 종료
  Wire.endTransmission();

  // 입력값 확인용
  Serial.print("온도 :");
  Serial.print(Temp);
  Serial.print(" 습도 :");
  Serial.println(Hum);
}

void setup() {
  /*--------------------------------------------*/

  /* Serial 확인 */

  Serial.begin(9600);

  /*--------------------------------------------*/

  /*--------------------------------------------*/

  /* DHT 확인 */

  // DHT테스트
  Serial.println(F("DHTxx test!"));
  // DHT센서 동작
  dht.begin();

  /*--------------------------------------------*/

  /*--------------------------------------------*/

  /* I2C 확인 */

  // I2C 통신 방식 마스터 지정
  Wire.begin();

  /*--------------------------------------------*/
}

void loop() {

  delay(2000);

  float t = Temperature_read();
  float h = Humidity_read();

  /*--------------------------------------------*/

  // 온습도 센서 읽을 수 있는지 확인
  if (isnan(Humidity_read()) || isnan(Temperature_read())) {
    Serial.println(F("DHT센서를 읽을 수 없음!"));
    return;
  }

  /*--------------------------------------------*/

  // 여기에 필요한 동작 추가
  // 예: 센서 데이터 읽기, 데이터 처리 등
  WriteData(address_01, t, h);
  WriteData(address_02, t, h);

  delay(1000);
}

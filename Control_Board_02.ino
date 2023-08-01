#include <Wire.h>
void setup() {
  // put your setup code here, to run once:
  Wire.begin(0x03);
  Wire.onRequest(handler);
}

void loop() {
  // put your main code here, to run repeatedly:
  handler();
  delay(1000);
}

void handler (){
  Wire.write(12);
}
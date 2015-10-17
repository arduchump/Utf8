
#include <Arduino.h>
#include <Utf8.h>

const char *gUtf8ZhString = "你好，我是中文！";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello, world!");
  Serial.println("Utf8 library testing ...");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(3000);
  Serial.print("Utf8 zh_CN string length : ");
  Serial.println(Utf8GetLength(gUtf8ZhString));
}

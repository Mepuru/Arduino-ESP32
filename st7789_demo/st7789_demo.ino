/*
 * 极简测试 — 确认 ESP32-S3 程序能否运行
 */
void setup() {
  Serial.begin(115200);
  delay(2000);  // 等串口连上

  Serial.println("\n=== ALIVE ===");
  Serial.println("If you see this, the sketch runs.");

  // 让 GPIO16 (新 BL 引脚) 闪烁 5 次
  pinMode(16, OUTPUT);
  for (int i = 0; i < 5; i++) {
    digitalWrite(16, HIGH);
    delay(500);
    digitalWrite(16, LOW);
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nBlink done");
}

void loop() {
  Serial.println("loop tick");
  delay(2000);
}

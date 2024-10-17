uint8_t GPIO_Pin = D8;
int a;
uint64_t time1;
void ICACHE_RAM_ATTR A();
void setup() {
  Serial.begin(115200);
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);

  attachInterrupt(digitalPinToInterrupt(D8), A, RISING);
  // attachInterrupt(digitalPinToInterrupt(D7), B, CHANGE);
  Serial.println("ad2");
}

void loop() {
  // Serial.println(a);
}
void A() {
  //  Serial.print("Stamp(ms): ");
  Serial.println(micros()-time1);
  if (digitalRead(D7)) {
    a++;
    time1=micros();
  }
    else {
    a--;
    time1=micros();
  }
}


#define diameter 72

double dis;
uint8_t GPIO_Pin = D8;
long int a=0;
uint64_t time1;
void ICACHE_RAM_ATTR A();
void setup() {
  Serial.begin(115200);
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);

  attachInterrupt(digitalPinToInterrupt(D8), A, RISING);
}

void loop() {
  dis=(float)a/400*diameter*3.141592/10;
  Serial.print("palse: ");
  Serial.println(a);
  Serial.print("distance: ");
  Serial.println(dis);
  delay(200);
}
void A() {
  //  Serial.print("Stamp(ms): ");
  // Serial.println(micros() - time1);
  if (digitalRead(D7)) {
    a++;
  } else {
    a--;
  }
  
  time1 = micros();
}

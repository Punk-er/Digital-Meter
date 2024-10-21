
#define diameter 73

double dis;
#define GPIO_Pin D3
#define GPIO_Pin2 D4
long int a=0;
uint64_t time1;
void ICACHE_RAM_ATTR A();
void setup() {
  Serial.begin(115200);
  pinMode(GPIO_Pin, INPUT);
  pinMode(GPIO_Pin2, INPUT);

  attachInterrupt(digitalPinToInterrupt(GPIO_Pin), A, RISING);
}

void loop() {
  dis=(double)a/400*diameter*3.141592/10;
  Serial.print("palse: ");
  Serial.println(a);
  Serial.print("distance: ");
  Serial.println(dis);
  delay(200);
}
void A() {
  //  Serial.print("Stamp(ms): ");
  // Serial.println(micros() - time1);
  if (digitalRead(GPIO_Pin2)) {
    a++;
  } else {
    a--;
  }
  
  time1 = micros();
}

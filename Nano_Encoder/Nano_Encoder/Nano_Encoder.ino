#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library
#include <SPI.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>

#define diameter 73
#define TFT_CS D1
#define TFT_DC D3
#define GPIO_Pin D4
#define GPIO_Pin2 D0
#define GPIO_Volium1 D2
#define GPIO_Volium2 D8
#define GPIO_BTN A0
double dis, velocity;
int level=0;
long int a = 0;
long int Volume = 0;
uint64_t time1;
uint16_t textx, texty, textWidth, textHeight;
void ICACHE_RAM_ATTR A();
void ICACHE_RAM_ATTR Vol();
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, 0);

class Button {
public:
  unsigned long int firstTime = 0, lastTime = 0;
  bool stat = false, firstpress = true, stillOnHold = false, Rising = true;
  int tapcounter = 0, holdTime = 800, tapTime = 20, tapGap = 400, pin;
  void (*callback_hold)(int), (*callback_tap)(int);
  Button(int pin, bool Rising) {
    this->pin = pin;
    this->Rising = Rising;
    if (Rising) {
      stat = false;
    } else {
      stat = true;
    }
  }
  // Hold Event(Hold time)  Tap Event(Tap count)
  void SetEvent(void (*callback_hold1)(int), void (*callback_tap1)(int)) {
    callback_hold = callback_hold1;
    callback_tap = callback_tap1;
  }
  // should repeat in a loop all the time in the main loop
  void Listen() {
    if (Rising) {
      if (stat != (analogRead(pin)>1000)) {
        stat = (analogRead(pin)>1000);
        if (stat)
          firstTime = millis();
        else {
          if (millis() - firstTime > tapTime && millis() - firstTime < holdTime) {
            tapcounter++;
            lastTime = millis();
          }
          if (stillOnHold && millis() - firstTime >= holdTime) {
            stillOnHold = false;
          }
        }
      } else {
        if (!stillOnHold && stat && millis() - firstTime >= holdTime) {
          callback_hold(millis() - firstTime);
          stillOnHold = true;
        }
        if (tapcounter != 0 and millis() - lastTime > tapGap) {
          callback_tap(tapcounter);
          tapcounter = 0;
        }
      }
    } else {
      if (stat != (analogRead(pin)>1000)) {
        stat = (analogRead(pin)>1000);
        if (!stat)
          firstTime = millis();
        else {
          if (millis() - firstTime > tapTime && millis() - firstTime < holdTime) {
            tapcounter++;
            lastTime = millis();
          }
          if (stillOnHold && millis() - firstTime >= holdTime) {
            stillOnHold = false;
          }
        }
      } else {
        if (!stillOnHold && !stat && millis() - firstTime >= holdTime) {
          callback_hold(millis() - firstTime);
          stillOnHold = true;
        }
        if (tapcounter != 0 and millis() - lastTime > tapGap) {
          callback_tap(tapcounter);
          tapcounter = 0;
        }
      }
    }
  }
};
Button btn(GPIO_BTN, true);
void on_hold(int time) {
  Serial.println(time);
}
void on_tab(int times) {
  Serial.println(times);
}
void setup() {
  Serial.begin(115200);
  pinMode(D6, OUTPUT);
  pinMode(GPIO_Pin, INPUT);
  pinMode(GPIO_Pin2, INPUT);
  pinMode(GPIO_Volium1, INPUT);
  pinMode(GPIO_Volium2, INPUT);
  pinMode(GPIO_BTN, INPUT);
  
  tft.initR(INITR_144GREENTAB);
  tft.fillScreen(ST7735_BLACK);
  btn.SetEvent(on_hold, on_tab);
  attachInterrupt(digitalPinToInterrupt(GPIO_Volium1), Vol, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_Pin), A, RISING);


  tft.setTextWrap(false);
  tft.fillScreen(ST7735_CYAN);
  tft.drawRoundRect(2, 2, tft.width() - 2, tft.height() - 2, 10, ST7735_YELLOW);
  tft.drawRoundRect(3, 3, tft.width() - 4, tft.height() - 4, 10, ST7735_YELLOW);
  tft.setCursor(15, tft.height() / 2 - 4);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.setFont(&FreeSerifBold9pt7b);
  tft.println("CHIST.ORG");
  delay(1000);
  tft.fillScreen(ST7735_BLACK);
  // tft.setCursor(2, 20);
  // tft.setTextColor(ST7735_BLUE);
  // tft.setTextSize(0.5);
  tft.setFont(&FreeMonoOblique9pt7b);
  // tft.println("Length:");
  // tft.setCursor(10, 40);

  updatablePrint("length:", 15, ST7735_RED, ST7735_BLACK, -1);

  updatablePrint((String)Volume + " cm", 35, ST7735_RED, ST7735_BLACK, -1);
  tft.setFont(&FreeSerifBold9pt7b);

  // delay(1000);
  // updatablePrint("me21hran", 35, ST7735_BLUE, ST7735_BLACK,0);
  // delay(1000);
  // updatablePrint("men", 35, ST7735_RED, ST7735_BLACK,1);
  // delay(1000);
}

void loop() {
  // Volume++;
  // updatablePrint((String)Volume+" cm", 35, ST7735_RED, ST7735_BLACK,0);
  // delay(5);
  // dis=(double)a/400*diameter*3.141592/10;
  // Serial.print("palse: ");
  // Serial.println(a);
  // Serial.print("distance: ");
  // Serial.println(dis);

  // Serial.print("Volume: ");
  // Serial.println(Volume);

  digitalWrite(D6, 1);
  delay(200);

  digitalWrite(D6, 0);
  delay(200);

  btn.Listen();
  if (millis() - time1 > 200) {
    Serial.println(digitalRead(GPIO_BTN));
    // Serial.println((((double)a / 400 * diameter * 3.141592 / 10) - dis));
    dis = (double)a / 400 * diameter * 3.141592 / 10;
    updatablePrint((String)dis + " cm", 35, ST7735_RED, ST7735_BLACK, 0);
    // updatablePrint((String)velocity+" interval", 65, ST7735_RED, ST7735_BLACK,0);
    time1 = millis();
  }

}

void updatablePrint(String text, int y, uint16_t text_color, uint16_t bg_color, int gravity) {
  int16_t x1, y1;

  tft.fillRect(textx, y - textHeight + 1, textWidth, textHeight, bg_color);
  tft.setTextColor(text_color);

  tft.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  if (gravity == -1) {
    tft.setCursor(0, y);
    textx = 0;
  } else if (gravity == 0) {
    tft.setCursor((tft.width() - textWidth) / 2, y);
    textx = (tft.width() - textWidth) / 2;
  } else if (gravity == 1) {
    tft.setCursor((tft.width() - textWidth), y);
    textx = (tft.width() - textWidth);
  }
  tft.print(text);
}
// void centerHorizontally(const String &textValue, int cursorXPos, int cursorYPos, uint16_t color) {
//   int16_t x1, y1;
//   uint16_t textWidth, textHeight;

//   tft.getTextBounds(textValue, 0, 0, &x1, &y1, &textWidth, &textHeight);
//   tft.fillRect(0, y1, tft.width(), textHeight, color);
//   tft.setCursor((tft.width() - textWidth) / 2, cursorYPos);

//   tft.print(textValue);
// }

void Vol() {
  if (digitalRead(GPIO_Volium2) == digitalRead(GPIO_Volium1)) {
    Volume++;
  } else {
    Volume--;
  }
  Serial.print("Volume: ");
  Serial.println(Volume);
}
void A() {
  if (digitalRead(GPIO_Pin2)) {
    a++;
  } else {
    a--;
  }
}

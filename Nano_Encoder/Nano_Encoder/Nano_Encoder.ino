#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library
#include <SPI.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>

#define diameter 73
#define TFT_CS 4
#define TFT_DC 5
#define GPIO_Pin 2
#define GPIO_Pin2 10
#define GPIO_Volium1 3
#define GPIO_Volium2 9
#define GPIO_BTN A0
#define relay_pin 6
#define BlinkTime 700
#define RefreshRate 50
double dis, velocity;
bool Motor_status = false, blink = true;
int edit_step = 0, LEVEL = 0;
long int Palse = 0;
long int Volume = 0;

int telo_CM = 0;
int telo_MM = 0;
int target_CM = 0;
int target_MM = 0;
int target_Count = 0;

uint64_t BlinkTimer;
uint64_t RefreshTimer;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, 0);
class LcdPrint {
public:
  int X = 0, Y = 0, W = 0, H = 0;
  String lastText = "";
  int16_t X_internal = 0, Y_internal = 0;
  uint16_t bg_c, lastTC;
  LcdPrint(int x, int y, int w, int h, uint16_t bg) {
    this->X = x;
    this->Y = y;
    this->W = w;
    this->H = h;
    this->bg_c = bg;
  }
  void Clear() {
    tft.setCursor(X_internal, Y_internal);
    tft.setTextColor(bg_c);
    tft.print(lastText);
    lastText = "";
  }
  void print(String text, int HGravity, int VGravity, uint16_t text_color) {
    if (text != lastText || lastTC != text_color) {
      int16_t a1, b1;
      uint16_t textWidth, textHeight;
      tft.setCursor(X_internal, Y_internal);
      tft.setTextColor(bg_c);
      tft.print(lastText);
      tft.setTextColor(text_color);
      lastTC = text_color;
      tft.getTextBounds(text, 0, 0, &a1, &b1, &textWidth, &textHeight);
      if (HGravity == -1) {
        if (VGravity == -1) {
          X_internal = X;
          Y_internal = Y + H;
        } else if (VGravity == 0) {
          X_internal = X;
          Y_internal = Y + (H / 2 + textHeight / 2);
        } else if (VGravity == 1) {
          X_internal = X;
          Y_internal = Y + textHeight;
        }
      } else if (HGravity == 0) {
        if (VGravity == -1) {
          X_internal = X + (W - textWidth) / 2;
          Y_internal = Y + H;
        } else if (VGravity == 0) {
          X_internal = X + (W - textWidth) / 2;
          Y_internal = Y + (H / 2 + textHeight / 2);
        } else if (VGravity == 1) {
          X_internal = X + (W - textWidth) / 2;
          Y_internal = Y + textHeight;
        }
      } else if (HGravity == 1) {
        if (VGravity == -1) {
          X_internal = X + (W - textWidth);
          Y_internal = Y + H;
        } else if (VGravity == 0) {
          X_internal = X + (W - textWidth);
          Y_internal = Y + (H / 2 + textHeight / 2);
        } else if (VGravity == 1) {
          X_internal = X + (W - textWidth);
          Y_internal = Y + textHeight;
        }
      }

      tft.setCursor(X_internal, Y_internal);
      tft.print(text);
      lastText = text;
    }
  }
};
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
      if (stat != (digitalRead(pin))) {
        stat = (digitalRead(pin));
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
      if (stat != (digitalRead(pin))) {
        stat = (digitalRead(pin));
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


LcdPrint telorancetext(5, 2, 65, 15, ST7735_BLACK);
LcdPrint telorance(5, 15, 118, 25, ST7735_BLACK);
LcdPrint Targettext(5, 41, 118, 15, ST7735_BLACK);
LcdPrint Targetlen(5, 55, 118, 25, ST7735_BLACK);
LcdPrint CountText(5, 84, 118, 15, ST7735_BLACK);
LcdPrint Count(5, 96, 118, 25, ST7735_BLACK);

LcdPrint Lengthtext(5, 5, 118, 10, ST7735_BLACK);
LcdPrint Length(5, 25, 118, 20, ST7735_BLACK);

// LcdPrint Lengthtext(69, 44, 59, 15, ST7735_BLACK);
// LcdPrint Length(69, 61, 59, 20, ST7735_BLACK);

// LcdPrint Lengthtext(69, 44, 59, 15, ST7735_BLACK);
// LcdPrint Length(69, 61, 59, 20, ST7735_BLACK);

Button btn(GPIO_BTN, true);
void on_hold(int time) {
  LEVEL++;
  if (LEVEL > 1) LEVEL = 0;
  if (LEVEL == 0) {
    edit_step = 0;
    tft.fillScreen(ST7735_BLACK);
    Length.Clear();
    Lengthtext.Clear();
    tft.drawRect(2, 1, 126, 38, ST7735_RED);
    tft.drawRect(3, 2, 124, 36, ST7735_RED);
    tft.drawRect(1, 40, 126, 38, ST7735_BLUE);
    tft.drawRect(2, 41, 124, 36, ST7735_BLUE);
    tft.drawRect(1, 80, 126, 38, ST7735_ORANGE);
    tft.drawRect(2, 81, 124, 36, ST7735_ORANGE);
    telorancetext.print("Telorance", -1, 0, ST7735_RED);
    Targettext.print("Target", -1, 0, ST7735_BLUE);
    CountText.print("Count", -1, 0, ST7735_ORANGE);
    telorance.print((String)telo_CM + "." + (String)telo_MM + " CM", 0, 0, ST7735_RED);
    Targetlen.print((String)target_CM + "." + (String)target_MM + " CM", 0, 0, ST7735_BLUE);
    Count.print((String)target_Count + " PCS", 0, 0, ST7735_ORANGE);

  } else if (LEVEL == 1) {
    tft.fillScreen(ST7735_BLACK);
    telorancetext.Clear();
    Targettext.Clear();
    CountText.Clear();
    telorance.Clear();
    Targetlen.Clear();
    Count.Clear();
    tft.drawRect(2, 1, 126, 50, ST7735_WHITE);
    tft.drawRect(3, 2, 124, 48, ST7735_WHITE);
    Lengthtext.print("Length:", 0, 0, ST7735_WHITE);
    Length.print((String)Palse, 0, 0, ST7735_WHITE);
  }
}
void on_tab(int times) {
  edit_step += times;
  if (edit_step > 4) edit_step = 0;
}
void setup() {
  btn.holdTime = 3000;
  Serial.begin(115200);
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
  tft.drawRoundRect(4, 4, tft.width() - 6, tft.height() - 6, 10, ST7735_YELLOW);
  tft.setCursor(15, tft.height() / 2 - 4);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.setFont(&FreeSerifBold9pt7b);
  tft.println("CHIST.ORG");
  delay(1000);
  tft.setFont(&FreeSerifItalic9pt7b);
  // tft.setTextSize(2);

  tft.fillScreen(ST7735_BLACK);
  tft.drawRect(2, 1, 126, 38, ST7735_RED);
  tft.drawRect(3, 2, 124, 36, ST7735_RED);
  tft.drawRect(1, 40, 126, 38, ST7735_BLUE);
  tft.drawRect(2, 41, 124, 36, ST7735_BLUE);
  tft.drawRect(1, 80, 126, 38, ST7735_ORANGE);
  tft.drawRect(2, 81, 124, 36, ST7735_ORANGE);
  telorancetext.print("Telorance", -1, 0, ST7735_RED);
  Targettext.print("Target", -1, 0, ST7735_BLUE);
  CountText.print("Count", -1, 0, ST7735_ORANGE);
}

void loop() {
  btn.Listen();
  if (LEVEL == 0 && millis() - BlinkTimer > BlinkTime) {
    blink = !blink;
    if (edit_step == 0 || edit_step == 1) {
      CountText.print("Count", -1, 0, ST7735_ORANGE);
      if (blink) {
        telorancetext.print("Telorance", -1, 0, ST7735_RED);
      } else {
        telorancetext.Clear();
      }

    } else if (edit_step == 2 || edit_step == 3) {
      telorancetext.print("Telorance", -1, 0, ST7735_RED);
      if (blink) {
        Targettext.print("Target", -1, 0, ST7735_BLUE);
      } else {
        Targettext.Clear();
      }

    } else if (edit_step == 4) {
      Targettext.print("Target", -1, 0, ST7735_BLUE);
      if (blink) {
        CountText.print("Count", -1, 0, ST7735_ORANGE);
      } else {
        CountText.Clear();
      }
    } else {
    }
    BlinkTimer = millis();
  }
  if (millis() - RefreshTimer > RefreshRate) {
    if (LEVEL == 0) {
      telorance.print((String)telo_CM + "." + (String)telo_MM + " CM", 0, 0, ST7735_RED);
      Targetlen.print((String)target_CM + "." + (String)target_MM + " CM", 0, 0, ST7735_BLUE);
      Count.print((String)target_Count + " PCS", 0, 0, ST7735_ORANGE);
    } else if (LEVEL == 1) {
      Length.print((String)Palse, 0, 0, ST7735_WHITE);
    }
    RefreshTimer = millis();
  }
}


void Vol() {
  if (LEVEL == 0) {
    if (digitalRead(GPIO_Volium2) == digitalRead(GPIO_Volium1)) {
      if (edit_step == 0) {
        telo_MM--;
        if (telo_MM < 0) telo_MM = 9;
      } else if (edit_step == 1) {
        telo_CM--;

      } else if (edit_step == 2) {
        target_MM--;
        if (target_MM < 0) target_MM = 9;
      } else if (edit_step == 3) {
        if (target_CM > 0) target_CM--;
      } else if (edit_step == 4) {
        if (target_Count > 0) target_Count--;
      }
    } else {
      if (edit_step == 0) {
        telo_MM++;
        if (telo_MM > 9) telo_MM = 0;
      } else if (edit_step == 1) {
        telo_CM++;
      } else if (edit_step == 2) {
        target_MM++;
        if (target_MM > 9) target_MM = 0;
      } else if (edit_step == 3) {
        target_CM++;
      } else if (edit_step == 4) {
        target_Count++;
      }
    }
  }
}
void A() {
  if (!edit_step) {
    if (digitalRead(GPIO_Pin2)) {
      Palse++;
    } else {
      Palse--;
    }
  }
}

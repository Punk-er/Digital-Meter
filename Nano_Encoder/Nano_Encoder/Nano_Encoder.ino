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
double dis, velocity;
bool Motor_status = false, blink = true;
int edit_step = 0, LEVEL = 0;
long int a = 0;
long int Volume = 0;
long int telo = 0;
long int target = 0;
long int target_Count = 0;
uint64_t time1;

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
LcdPrint lengtext(5, 2, 65, 15, ST7735_BLACK);
LcdPrint length(5, 2, 118, 35, ST7735_BLACK);
LcdPrint Targettext(5, 83, 118, 20, ST7735_BLACK);
LcdPrint Targetlen(5, 102, 118, 20, ST7735_BLACK);
LcdPrint telorancetext(69, 44, 59, 15, ST7735_BLACK);
LcdPrint telorance(69, 61, 59, 20, ST7735_BLACK);
LcdPrint CountText(5, 44, 59, 15, ST7735_BLACK);
LcdPrint Count(5, 61, 59, 20, ST7735_BLACK);

// LcdPrint leng7(0, 0, 65, 20, ST7735_BLACK);
// void mainwin(int Update_level) {

//   if (Update_level >= 1) {
//     // lengtext.print("Length:", -1, 0, ST7735_RED);
//     tft.setTextSize(1.5);
//     length.print((String)dis + " cm", 0, 0, ST7735_RED);
//     tft.setTextSize(1);
//     // Statustext.print("State:", -1, 0, ST7735_BLUE);
//     // Status.print("OFF", 0, 0, ST7735_BLUE);
//     telorancetext.print("Tole:", -1, 0, ST7735_BLUE);
//     telorance.print((String)telo + " cm", 0, 0, ST7735_BLUE);
//     Targettext.print("Target:", -1, 0, ST7735_ORANGE);
//     Targetlen.print((String)target + " cm", 0, 0, ST7735_ORANGE);
//   }
//   if (Update_level >= 2) {
//     tft.drawRect(2, 1, 126, 40, ST7735_RED);
//     tft.drawRect(3, 2, 124, 38, ST7735_RED);
//     tft.drawRect(1, 42, 62, 40, ST7735_BLUE);
//     tft.drawRect(64, 43, 62, 38, ST7735_BLUE);
//     tft.drawRect(1, 83, 126, 40, ST7735_ORANGE);
//     tft.drawRect(2, 84, 124, 38, ST7735_ORANGE);
//   }
// }
Button btn(GPIO_BTN, true);
void on_hold(int time) {
  // LEVEL++;
  if (LEVEL > 1) LEVEL = 0;

  Serial.print("holed ");
  Serial.println(time);
}
void on_tab(int times) {
  edit_step++;
  if (edit_step > 2) edit_step = 0;

  Serial.print("tapped ");
  Serial.println(times);
}
void setup() {
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
  tft.setCursor(15, tft.height() / 2 - 4);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.setFont(&FreeSerifBold9pt7b);
  tft.println("CHIST.ORG");
  delay(1000);
  tft.setFont(&FreeSerifItalic9pt7b);
  tft.setTextSize(2);

  tft.fillScreen(ST7735_BLACK);
  tft.drawRect(2, 1, 126, 40, ST7735_RED);
  tft.drawRect(3, 2, 124, 38, ST7735_RED);
  tft.drawRect(1, 42, 126, 40, ST7735_BLUE);
  tft.drawRect(2, 43, 124, 38, ST7735_BLUE);
  tft.drawRect(1, 83, 126, 40, ST7735_ORANGE);
  tft.drawRect(2, 84, 124, 38, ST7735_ORANGE);
  Targettext.X=5;
  Targettext.Y=40;
  Targettext.H=40;
  Targettext.W=128;
  Targettext.print("Telorance", -1, 0, ST7735_RED);
    Targettext.print("Target:", -1, 0, ST7735_ORANGE);

  // tft.setCursor(2, 20);
  // tft.setTextColor(ST7735_BLUE);
  // tft.setTextSize(0.5);
  // tft.setFont(&FreeMonoOblique9pt7b);
  // tft.println("Length:");
  // tft.setCursor(10, 40);

  // updatablePrint("length:", 15, ST7735_RED, ST7735_BLACK, -1);

  // updatablePrint((String)Volume + " cm", 35, ST7735_RED, ST7735_BLACK, -1);
  // tft.setFont(&FreeSerifBold9pt7b);

  // delay(1000);
  // updatablePrint("me21hran", 35, ST7735_BLUE, ST7735_BLACK,0);
  // delay(1000);
  // updatablePrint("men", 35, ST7735_RED, ST7735_BLACK,1);
  // delay(1000);


  // leng.Print("awdad", -1, -1, ST7735_RED);
  // leng.Clear();
  // mainwin(2);
}

void loop() {
  btn.Listen();
  if (LEVEL == 0 && millis() - time1 > 500) {
    blink = !blink;
    if (edit_step == 0) {
      if (blink) {
        Targettext.print("Telorance", -1, 0, ST7735_RED);
      } else {
        Targettext.Clear();
      }
    } else if (edit_step == 1) {

    } else if (edit_step == 2) {

    } else {
    }
    time1 = millis();
  }
  // if (edit_step == 1 && millis() - time1 > 500) {
  //   blink = !blink;
  //   if (blink) {
  //     telorancetext.Clear();
  //   } else {
  //     mainwin(1);
  //   }

  //   time1 = millis();
  // } else if (edit_step == 2 && millis() - time1 > 500) {
  //   blink = !blink;
  //   if (blink) {
  //     Targettext.Clear();
  //   } else {
  //     mainwin(1);
  //   }

  //   time1 = millis();
  // }

  if (LEVEL == 0 && millis() - time1 > 200) {
    // Serial.println((((double)a / 400 * diameter * 3.141592 / 10) - dis));
    dis = (double)a / 400 * diameter * 3.141592 / 10;


    time1 = millis();
  }
}


void Vol() {
  if (LEVEL == 0) {
    if (digitalRead(GPIO_Volium2) == digitalRead(GPIO_Volium1)) {
      if (edit_step == 0) {
        telo--;
      } else if (edit_step == 1) {
        target--;
      } else if (edit_step == 2) {
        target_Count--;
      }
    } else {
      if (edit_step == 0) {
        telo++;
      } else if (edit_step == 1) {
        target++;
      } else if (edit_step == 2) {
        target_Count++;
      }
    }
  }
  Serial.print("Volume: ");
  Serial.println(Volume);
}
void A() {
  if (!edit_step) {
    if (digitalRead(GPIO_Pin2)) {
      a++;
    } else {
      a--;
    }
  }
}

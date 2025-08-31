#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== Display =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== Pins =====
#define PIN_LID     2
#define PIN_LED     3
#define PIN_BTN_ON  4
#define PIN_BTN_PREV 5
#define PIN_BTN_NEXT 6
#define PIN_BTN_SET 7
#define PIN_FAN     8
#define PIN_BUZZER  9
#define PIN_REDLED  11
#define PIN_GREENLED 12
#define PIN_THERM   A0

// ===== Timer presets =====
#define MAX_PRESETS 10
uint16_t presets[MAX_PRESETS] = {10, 30, 60, 120, 200, 300, 600, 900, 1200, 1800};
int currentPreset = 0;

// ===== States =====
enum Mode { MODE_IDLE, MODE_RUNNING, MODE_DONE, MODE_EDIT, MODE_MENU };
Mode mode = MODE_IDLE;

unsigned long timerStart = 0;
uint16_t timerDuration = 0;
bool lidEnabled = true;
int tempLimit = 60; // °C
int fanMode = 1;    // 0=Always, 1=Timer only, 2=Off

// ===== Buttons =====
bool lastOn=false, lastPrev=false, lastNext=false, lastSet=false;

// ===== Buzzer Patterns =====
enum BuzzPattern { BUZZ_SILENT, BUZZ_SINGLE, BUZZ_DOUBLE, BUZZ_TRIPLE, BUZZ_LONG, BUZZ_CHIRP };
BuzzPattern buzzSetting = BUZZ_TRIPLE;

void beep(BuzzPattern pattern) {
  if (pattern == BUZZ_SILENT) return;
  switch (pattern) {
    case BUZZ_SINGLE:
      tone(PIN_BUZZER, 2000, 200); delay(250); break;
    case BUZZ_DOUBLE:
      tone(PIN_BUZZER, 2000, 200); delay(300);
      tone(PIN_BUZZER, 2000, 200); delay(300); break;
    case BUZZ_TRIPLE:
      for (int i=0;i<3;i++){ tone(PIN_BUZZER,2000,150); delay(250);} break;
    case BUZZ_LONG:
      tone(PIN_BUZZER, 1500, 800); delay(900); break;
    case BUZZ_CHIRP:
      for (int f=1000; f<=3000; f+=500) { tone(PIN_BUZZER,f,80); delay(100);} break;
    default: break;
  }
}

// ===== Temperature (simplified, assume 10k NTC beta ~3950) =====
float readTemp() {
  int adc = analogRead(PIN_THERM);
  float R = 10000.0 * (1023.0 / adc - 1);
  float tempK = 1.0 / (1.0/298.15 + 1.0/3950.0 * log(R/10000.0));
  return tempK - 273.15;
}

// ===== Setup =====
void setup() {
  pinMode(PIN_LID, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_REDLED, OUTPUT);
  pinMode(PIN_GREENLED, OUTPUT);
  pinMode(PIN_BTN_ON, INPUT_PULLUP);
  pinMode(PIN_BTN_PREV, INPUT_PULLUP);
  pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BTN_SET, INPUT_PULLUP);

  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_FAN, LOW);
  digitalWrite(PIN_REDLED, LOW);
  digitalWrite(PIN_GREENLED, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("UV Box Ready");
  display.display();
}

// ===== Draw UI =====
void drawUI() {
  display.clearDisplay();
  if (mode == MODE_RUNNING) {
    int remain = timerDuration - (millis() - timerStart)/1000;
    display.setCursor(0,0); display.print("RUN "); display.print(remain); display.println("s");
  } else if (mode == MODE_DONE) {
    display.setCursor(0,0); display.print("DONE "); display.print(timerDuration); display.println("s");
  } else if (mode == MODE_EDIT) {
    display.setCursor(0,0); display.print("Edit: "); display.print(presets[currentPreset]); display.println("s");
  } else if (mode == MODE_MENU) {
    display.setCursor(0,0); display.println("Settings:");
    display.setCursor(0,10); display.print("Buzzer: "); 
    switch(buzzSetting){ case BUZZ_SILENT: display.print("Silent"); break;
    case BUZZ_SINGLE: display.print("Single"); break;
    case BUZZ_DOUBLE: display.print("Double"); break;
    case BUZZ_TRIPLE: display.print("Triple"); break;
    case BUZZ_LONG: display.print("Long"); break;
    case BUZZ_CHIRP: display.print("Chirp"); break;}
    display.setCursor(0,20); display.print("Lid: "); display.print(lidEnabled?"Enabled":"Disabled");
  } else {
    display.setCursor(0,0); display.print("Idle Preset: "); display.print(presets[currentPreset]); display.println("s");
  }
  display.display();
}

// ===== Loop =====
void loop() {
  // Read buttons
  bool on = !digitalRead(PIN_BTN_ON);
  bool prev = !digitalRead(PIN_BTN_PREV);
  bool next = !digitalRead(PIN_BTN_NEXT);
  bool set = !digitalRead(PIN_BTN_SET);

  // Example: toggle modes
  if (on && !lastOn) {
    if (mode == MODE_IDLE) { mode=MODE_RUNNING; timerStart=millis(); timerDuration=presets[currentPreset]; digitalWrite(PIN_LED,HIGH); digitalWrite(PIN_REDLED,HIGH); if(fanMode!=2) digitalWrite(PIN_FAN,HIGH);}
    else if (mode == MODE_RUNNING) { mode=MODE_IDLE; digitalWrite(PIN_LED,LOW); digitalWrite(PIN_REDLED,LOW); digitalWrite(PIN_FAN,LOW);}
    else if (mode == MODE_DONE) { mode=MODE_IDLE; digitalWrite(PIN_GREENLED,LOW);}
  }
  lastOn=on; lastPrev=prev; lastNext=next; lastSet=set;

  // Timer finished?
  if (mode==MODE_RUNNING && (millis()-timerStart)/1000 >= timerDuration) {
    mode=MODE_DONE;
    digitalWrite(PIN_LED,LOW); digitalWrite(PIN_REDLED,LOW); digitalWrite(PIN_GREENLED,HIGH);
    if(fanMode==1) digitalWrite(PIN_FAN,LOW);
    beep(buzzSetting);
  }

  drawUI();
  delay(50);
}

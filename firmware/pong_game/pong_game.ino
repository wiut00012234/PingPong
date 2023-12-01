/*
  Game on the addressable LED strip "Pong"
   - Press the button when the ball hits your zone
   - The closer to the edge of the tape, the stronger the rebound will be
   - Pressed outside of your zone - lost
   - Missed - lost
*/

#define LED_PIN 2     // Led strip pin
#define LED_NUM 60   // number of LEDs
#define LED_BR 250     // brightness

#define B1_PIN 3      // pin of button 1
#define B2_PIN 4      // pin of button 2
#define BUZZ_PIN 5    // buzzer pin

#define ZONE_SIZE 10  // size of the zone
#define MIN_SPEED 5   // minimum speed
#define MAX_SPEED 20  // maximum speed
#define WIN_SCORE 5   // winning score

// =============================================
#include "FastLED.h"
CRGB leds[LED_NUM];

// ========== Button ==========
// мини класс кнопки
#define BTN_DEB 50    // дебаунс, мс
struct Button {
  public:
    Button (byte pin) {
      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
    }
    bool click() {
      bool btnState = digitalRead(_pin);
      if (!btnState && !_flag && millis() - _tmr >= BTN_DEB) {
        _flag = true;
        _tmr = millis();
        return true;
      }
      if (btnState && _flag && millis() - _tmr >= BTN_DEB) {
        _flag = false;
        _tmr = millis();
      }
      return false;
    }
    uint32_t _tmr;
    byte _pin;
    bool _flag;
};

Button b1(B1_PIN);
Button b2(B2_PIN);

// ============ SETUP =============
void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUM);
  FastLED.setBrightness(LED_BR);
  pinMode(BUZZ_PIN, OUTPUT);
  newGame();
}

// ========== VARIABLES ==========
int pos = 0;
int spd;
byte score1 = 0, score2 = 0;
uint32_t tmr;

// ============= LOOP =============
void loop() {
  poolButtons();
  gameRoutine();
  
  if (Serial.available() > 0) {
    char receivedChar = Serial.read();
    if (receivedChar == '1') {
      handlePlayerAction(1);
    } else if (receivedChar == '2') {
      handlePlayerAction(2);
    }
  }
}

// ========= POLLING BUTTONS =========
void poolButtons() {
  if (b1.click()) {                             // произошёл клик игрока 1
    if (pos >= ZONE_SIZE * 10) gameOver(0);     // мячик вне зоны 1 игрока - проиграл
    else {                                      // мячик в зоне - отбил
      tone(BUZZ_PIN, 1200, 60);
      spd = map(pos, ZONE_SIZE * 10, 0, MIN_SPEED, MAX_SPEED); // меняем скорость
    }
  }

  // similar for player 2
  if (b2.click()) {
    if (pos < (LED_NUM - ZONE_SIZE) * 10) gameOver(1);
    else {
      tone(BUZZ_PIN, 1200, 60);
      spd = map(pos, (LED_NUM - ZONE_SIZE) * 10, LED_NUM * 10, -MIN_SPEED, -MAX_SPEED);
    }
  }
}

void handlePlayerAction(int player) {
  // Player 1 action
  if (player == 1) {
    if (pos >= ZONE_SIZE * 10) gameOver(0);
    else {
      tone(BUZZ_PIN, 1200, 60);
      spd = map(pos, ZONE_SIZE * 10, 0, MIN_SPEED, MAX_SPEED);
    }
  }

  // Player 2 action
  if (player == 2) {
    if (pos < (LED_NUM - ZONE_SIZE) * 10) gameOver(1);
    else {
      tone(BUZZ_PIN, 1200, 60);
      spd = map(pos, (LED_NUM - ZONE_SIZE) * 10, LED_NUM * 10, -MIN_SPEED, -MAX_SPEED);
    }
  }
}

// ========= FILLING ZONES =========
void fillZones(CRGB color1, CRGB color2) {
  // заливаем концы ленты переданными цветами
  for (int i = 0; i < ZONE_SIZE; i++) {
    leds[i] = color1;
    leds[LED_NUM - i - 1] = color2;
  }
}

// ========= FLASHING AND BEEPING =========
// (color 1, color 2, purity, delay time)
void blinkTone(CRGB color1, CRGB color2, int freq, int del) {
  fillZones(color1, color2);    // fill zones
  FastLED.show();               // show
  tone(BUZZ_PIN, freq);         // buzz
  delay(del);                   // wait
  noTone(BUZZ_PIN);             // don't buzz
  fillZones(0, 0);              // turn off zones
  FastLED.show();               // show
  delay(del);                   // wait
}

// =========== LOSS ===========
// (player number, 0 or 1)
void gameOver(byte player) {
  newRound();    // new round

  if (player == 0) {
    score2++;
    if (score2 == WIN_SCORE) {  // won player 2
      score1 = score2 = 0;      // reset scores
      // winning beep beep beep for player 2
      blinkTone(CRGB::Black, CRGB::Green, 600, 200);
      blinkTone(CRGB::Black, CRGB::Green, 600, 200);
      blinkTone(CRGB::Black, CRGB::Green, 600, 200);
      delay(500);
      newGame();  // new game
    } else blinkTone(CRGB::Red, CRGB::Green, 200, 500);   // red beep for player 1
  } else {
    score1++;
    if (score1 == WIN_SCORE) {  // won player 1
      score1 = score2 = 0;
      blinkTone(CRGB::Green, CRGB::Black, 600, 200);
      blinkTone(CRGB::Green, CRGB::Black, 600, 200);
      blinkTone(CRGB::Green, CRGB::Black, 600, 200);
      delay(500);
      newGame();  // new game
    } else blinkTone(CRGB::Green, CRGB::Red, 200, 500);
  }
}

// ============== NEW GAME ==============
void newGame() {
  blinkTone(CRGB::Red, CRGB::Red, 300, 300);
  blinkTone(CRGB::Yellow, CRGB::Yellow, 300, 300);
  blinkTone(CRGB::Green, CRGB::Green, 600, 300);
  fillZones(CRGB::Green, CRGB::Green);
  FastLED.show();
  randomSeed(millis()); // making random numbers more random
  newRound();
}

// ============== NEW ROUND ==============
void newRound() {
  spd = random(0, 2) ? MIN_SPEED : -MIN_SPEED;  // random direction
  pos = (LED_NUM * 10) / 2;     // to the center of the strip
}

// ============== GAME  ==============
void gameRoutine() {
  if (millis() - tmr >= 10) {   // every 10 ms
    tmr = millis();
    pos += spd;     // move the ball

    if (pos < 0) {  // flew out to the left
      gameOver(0);  // player 1 lost
      return;       // exit
    }

    if (pos >= LED_NUM * 10) {  // flew out to the right
      gameOver(1);              // player 2 lost
      return;                   // exit
    }

    FastLED.clear();
    fillZones(CRGB::Green, CRGB::Green);  // show zones
    leds[pos / 10] = CRGB::Cyan;          // draw the ball
    FastLED.show();
  }
}
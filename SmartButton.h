#pragma once

#include <Arduino.h>

#define DEBOUNCE_DELAY 10
#define CLICK_MAX_DURATION 500
#define MULTI_CLICK_MAX_DELAY 400

#define buttonCallback(x) [](x) -> void

class SmartButton {
  private:
  byte pin;
  byte inputType;
  bool dbKnownState = true;
  bool readState = false;
  unsigned long readStateMillis = 0;
  bool value = false;
  bool knownValue = false;
  bool checkClick = false;
  unsigned long checkClickMillis = 0;
  bool checkMultiClick = false;
  unsigned long checkMultiClickMillis = 0;
  int clicks = 0;

  void (*onPressCb)();
  void (*onReleaseCb)();
  void (*onClickCb)(byte);

  public:
  SmartButton(byte pin, byte type = INPUT_PULLUP);
  void onPress(void (*callback)());
  void onRelease(void (*callback)());
  void onClick(void (*callback)(byte));
  void init();
  void handle();
};

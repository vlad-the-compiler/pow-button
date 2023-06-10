#include "SmartButton.h"

SmartButton::SmartButton(byte pin, byte type = INPUT_PULLUP) {
  this->pin = pin;
  inputType = type;
}

void SmartButton::onPress(void (*callback)()) {
  onPressCb = callback;
}

void SmartButton::onRelease(void (*callback)()) {
  onReleaseCb = callback;
}

void SmartButton::onClick(void (*callback)(byte)) {
  onClickCb = callback;
}

void SmartButton::init() {
  pinMode(pin, inputType);
}

void SmartButton::handle() {
  bool pinValue = digitalRead(pin);

  // Debounce
  if (pinValue != dbKnownState) {
    if (!readState) {
      readState = true;
      readStateMillis = millis();
      dbKnownState = pinValue;
    }
  }

  // Read value
  if (readState) {
    if (millis() - readStateMillis >= DEBOUNCE_DELAY) {
      value = !pinValue;
      readState = false;
    }
  }

  // Value changes
  if (value != knownValue) {
    if (value) {
      checkClick = true;
      checkClickMillis = millis();
    } else {
      if (checkClick) {
        clicks++;
        checkClick = false;
        checkMultiClick = true;
        checkMultiClickMillis = millis();
      } else {
        if (onReleaseCb) {
          onReleaseCb();
        }
      }
    }
  }
  knownValue = value;

  if (checkClick) {
    if (millis() - checkClickMillis >= CLICK_MAX_DURATION) {
        if (onPressCb) {
          onPressCb();
        }
      checkClick = false;
    }
  }

  if (checkMultiClick) {
    if (millis() - checkMultiClickMillis >= MULTI_CLICK_MAX_DELAY) {
      if (onClickCb) {
        onClickCb(clicks);
      }
      clicks = 0;
      checkMultiClick = false;
    }
  }
}

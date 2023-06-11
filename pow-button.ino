#include <avr/pgmspace.h>
#include <EEPROM.h>

#include "SmartButton.h"
#include "Animated.h"
#include "animation.h"

// User config; adjust to suit your needs
// Whether your front panel has a reset switch
#define HAVE_RESET_SW false
// Whether your front panel has a HDD activity LED separated from the main power indicator
#define SEPARATE_HDD_LED false
// Base brightness for your power indicator LED; gets scaled by the on-device brignthess setting
#define POWER_LED_BASE_BRIGHTNESS 255
// Base brightness for your HDD activity indicator LED; gets scaled by the on-device brignthess setting
// Front panels with single buttons that incorporate both power and HDD indicators may require a reduction of this value
#define HDD_LED_BASE_BRIGHTNESS 16

// Hardware config - Panel side
#define POWER_LED_PIN 5
#define HDD_LED_PIN 6
#define POWER_SW_SENSE_PIN 7
#define RESET_SW_SENSE_PIN 8
// Hardware config - Motherboard side
#define POWER_SW_OUT_PIN 3
#define RESET_SW_OUT_PIN 4
#define POWER_SENSE_PIN A0
#define POWER_SENSE_OPTO_EMITTER_PIN A1
#define HDD_SENSE_PIN A2
#define HDD_SENSE_OPTO_EMITTER_PIN A3

// Output config
#define SIMULATED_CLICK_DURATION 100

// Input interpretation config
#define POWER_STATE_DECISION_MILLIS 1000
#define WORKING_STATE_TEST_TICKS 100
#define WORKING_STATE_PASS_TICKS 25

// Software constants
#define MODE_SMART 0
#define MODE_PASSTHROUGH 1

// Configuration variables
byte brightness = 255;
byte mode = MODE_SMART;
byte modeToApply;

SmartButton powerSwitch(POWER_SW_SENSE_PIN, INPUT_PULLUP);

Animated powerLed;

bool isOn = false;
bool isWorking = false;
bool isStandby = false;
bool checkPowerInLow = false;
unsigned long checkPowerInLowMillis;
bool checkPowerInHigh = false;
unsigned long checkPowerInHighMillis;
int currentWorkingStateTicks = 0;
int currentWorkingStatePasses = 0;
bool exitSetupMode = false;
bool setupMode = false;

void applyStateAnimation() {
  if (isOn) {
    if (isWorking) {
      powerLed.startAnimation(ani_working);
    } else if (isStandby) {
      powerLed.startAnimation(ani_standby);
    } else {
      powerLed.startAnimation(ani_on);
    }
  } else {
    powerLed.startAnimation(ani_off);
  }
}

void simulatePanelButton(byte button, bool pressed) {
  if (pressed) {
    digitalWrite(button, LOW);
    pinMode(button, OUTPUT);
  } else {
    pinMode(button, INPUT);
  }
}

void setup() {
  pinMode(POWER_LED_PIN, OUTPUT);
  analogWrite(POWER_LED_PIN, 0);
  pinMode(HDD_LED_PIN, OUTPUT);
  analogWrite(HDD_LED_PIN, 0);
  // POWER_SW_SENSE_PIN already configured in SmartButton declaration
  pinMode(RESET_SW_SENSE_PIN, INPUT_PULLUP);

  pinMode(POWER_SW_OUT_PIN, INPUT);
  pinMode(RESET_SW_OUT_PIN, INPUT);

  pinMode(POWER_SENSE_PIN, INPUT_PULLUP);
  digitalWrite(POWER_SENSE_OPTO_EMITTER_PIN, LOW);
  pinMode(POWER_SENSE_OPTO_EMITTER_PIN, OUTPUT);

  pinMode(HDD_SENSE_PIN, INPUT_PULLUP);
  digitalWrite(HDD_SENSE_OPTO_EMITTER_PIN, LOW);
  pinMode(HDD_SENSE_OPTO_EMITTER_PIN, OUTPUT);

  Serial.begin(115200);
  // Check magic '1KD'
  if ((EEPROM[0] == '1') && (EEPROM[1] == 'K') && (EEPROM[2] == 'D')) {
    // Load config
    brightness = EEPROM[3];
    mode = EEPROM[4];
    Serial.println(F("*** Config ***"));
    Serial.print(F("Brightness = "));
    Serial.println(brightness);
    Serial.print(F("Mode = "));
    if (mode == MODE_SMART) {
      Serial.println(F("smart"));
    } else {
      Serial.println(F("passthrough"));
    }
  } else {
    Serial.println(F("EEPROM format error, formatting"));
    // Format
    // Magic
    EEPROM[0] = '1';
    EEPROM[1] = 'K';
    EEPROM[2] = 'D';
    // Brightness
    EEPROM[3] = brightness;
    EEPROM[4] = mode;
  }

  powerSwitch.init();
  powerSwitch.onPress(
    buttonCallback() {
      Serial.println(F("Power Switch Pressed"));
      if (setupMode) {
        Serial.println(F("Exiting Setup mode, saving data"));
        powerLed.startAnimation(ani_saveSetup);
        exitSetupMode = true;
      } else {
        simulatePanelButton(POWER_SW_OUT_PIN, true);
      }
    }
  );
  powerSwitch.onRelease(
    buttonCallback() {
      Serial.println(F("Power Switch Released"));
      simulatePanelButton(POWER_SW_OUT_PIN, false);
    }
  );
  powerSwitch.onClick(
    buttonCallback(byte clicks) {
      Serial.print(F("Power Switch Clicked "));
      Serial.print(clicks);
      Serial.println(F(" times"));
      if (setupMode) {
        if (clicks == 1) {
          if (brightness == 255) {
            brightness = 31;
          } else {
            brightness += 32;
          }
          powerLed.startAnimation(ani_brightness);
          Serial.print(F("Brightness = "));
          Serial.println(brightness);
        }
        // Mnemonic for the 2 syllables in "[Smart] [mode]"
        if (clicks == 2) {
          modeToApply = MODE_SMART;
          powerLed.startAnimation(ani_smartMode);
          Serial.println(F("Smart mode"));
        }
        // Mnemonic for the 3 syllables in "[Pass][through] [mode]"
        if (clicks == 3) {
          modeToApply = MODE_PASSTHROUGH;
          powerLed.startAnimation(ani_passthroughMode);
          Serial.println(F("Passthrough mode"));
        }
      } else {
        if (clicks == 3) {
          if (!HAVE_RESET_SW) {
            if (isOn) {
              if (!isStandby) {
                Serial.println(F("Issuing Reset signal"));
                simulatePanelButton(RESET_SW_OUT_PIN, true);
                delay(SIMULATED_CLICK_DURATION);
                simulatePanelButton(RESET_SW_OUT_PIN, false);
                powerLed.startAnimation(ani_restart);
              }
            }
          }
        }
        if (clicks == 1) {
          Serial.println(F("Issuing Power signal"));
          simulatePanelButton(POWER_SW_OUT_PIN, true);
          delay(SIMULATED_CLICK_DURATION);
          simulatePanelButton(POWER_SW_OUT_PIN, false);
        }
        if (clicks == 5) {
          Serial.println(F("Entering Setup mode"));
          powerLed.startAnimation(ani_setup);
          analogWrite(HDD_LED_PIN, 0);
          modeToApply = mode;
          mode = MODE_SMART;
          setupMode = true;
        }
      }
    }
  );
}

void loop() {
  // Handle switches
  powerSwitch.handle();
  if (mode == MODE_PASSTHROUGH) {
    simulatePanelButton(RESET_SW_OUT_PIN, !digitalRead(RESET_SW_SENSE_PIN));
  } else {
    if (HAVE_RESET_SW) {
      simulatePanelButton(RESET_SW_OUT_PIN, !digitalRead(RESET_SW_SENSE_PIN));
    }
  }

  // Power state detection
  if (!setupMode) {
    if (checkPowerInLow) {
      if (millis() - checkPowerInLowMillis >= POWER_STATE_DECISION_MILLIS) {
        Serial.println(F("Power In Low check delay finished, Power state is Off"));
        powerLed.startAnimation(ani_off);
        isOn = false;
        isWorking = false;
        isStandby = false;
        checkPowerInLow = false;
      }
    }
    if (checkPowerInHigh) {
      if (millis() - checkPowerInHighMillis >= POWER_STATE_DECISION_MILLIS) {
        Serial.println(F("Power In High check delay finished, Power state is On"));
        powerLed.startAnimation(ani_on);
        isOn = true;
        isWorking = false;
        isStandby = false;
        checkPowerInHigh = false;
      }
    }
    if (isOn) {
      if (digitalRead(POWER_SENSE_PIN)) {
        if (!checkPowerInLow) {
          checkPowerInLowMillis = millis();
          checkPowerInLow = true;
          checkPowerInHigh = false;
        }
      } else {
        if (checkPowerInLow) {
          checkPowerInLow = false;
          if (!isStandby) {
            Serial.println(F("Power In is strobing, Power state is Standby"));
            // Enqueue instead of override in order to get a smooth transition from any active HDD Working animation
            powerLed.enqueueAnimation(ani_standby);
            isStandby = true;
            isWorking = false;
          }
        }
        if (isStandby) {
          if (!checkPowerInHigh) {
            checkPowerInHighMillis = millis();
            checkPowerInHigh = true;
          }
        }
      }
    } else {
      if (!digitalRead(POWER_SENSE_PIN)) {
        Serial.println(F("Power In High, Power state is On"));
        powerLed.startAnimation(ani_on);
        isOn = true;
        isWorking = false;
      }
    }
  }

  // Get current brightness levels
  byte resolvedPowerLedBrightness = (uint16_t)(brightness * POWER_LED_BASE_BRIGHTNESS) >> 8;
  byte resolvedHddLedBrightness = (uint16_t)(brightness * HDD_LED_BASE_BRIGHTNESS) >> 8;

  // Animations + Other animation-synced routines
  if (shouldRunAnimations()) {
    // Animations
    byte powerLedValue = map(powerLed.runAnimation(), 0, 255, 0, resolvedPowerLedBrightness);
    if (mode == MODE_SMART) {
      analogWrite(POWER_LED_PIN, powerLedValue);
    }
    // Working state detection
    if (!SEPARATE_HDD_LED) {
      if (!setupMode) {
        if (isOn) {
          if (!isStandby) {
            currentWorkingStateTicks++;
            if (!digitalRead(HDD_SENSE_PIN)) {
              currentWorkingStatePasses++;
            }
            if (currentWorkingStateTicks >= WORKING_STATE_TEST_TICKS) {
              if (!isWorking) {
                if (currentWorkingStatePasses >= WORKING_STATE_PASS_TICKS) {
                  Serial.println(F("HDD activity high, setting Working state"));
                  powerLed.startAnimation(ani_working);
                  isWorking = true;
                }
              } else {
                if (currentWorkingStatePasses < WORKING_STATE_PASS_TICKS) {
                  Serial.println(F("HDD activity low, reverting Working state"));
                  powerLed.startAnimation(ani_on);
                  isWorking = false;
                }
              }
              currentWorkingStateTicks = 0;
              currentWorkingStatePasses = 0;
            }
          }
        }
      }
    }
  }

  // Handle LEDs
  if (mode == MODE_PASSTHROUGH) {
    analogWrite(POWER_LED_PIN, (!digitalRead(POWER_SENSE_PIN)) * resolvedPowerLedBrightness);
    analogWrite(HDD_LED_PIN, (!digitalRead(HDD_SENSE_PIN)) * resolvedHddLedBrightness);
  } else {
    if (SEPARATE_HDD_LED) {
      analogWrite(HDD_LED_PIN, (!digitalRead(HDD_SENSE_PIN)) * resolvedHddLedBrightness);
    } else {
      analogWrite(HDD_LED_PIN, 0);
    }
  }

  // Restore animations and save when exiting setup mode
  if (exitSetupMode) {
    if (!powerLed.animating()) {
      applyStateAnimation();
      mode = modeToApply;
      EEPROM[3] = brightness;
      EEPROM[4] = mode;
      Serial.println(F("Data saved"));
      setupMode = false;
      exitSetupMode = false;
    }
  }
}

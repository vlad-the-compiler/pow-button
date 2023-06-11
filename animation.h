#pragma once

#include <avr/pgmspace.h>

#include "animation_const.h"

const float ani_on[] PROGMEM = {
  1,
  255, 1,
  ANIMATION_STOP
};

const float ani_off[] PROGMEM = {
  1,
  0, 1.5,
  ANIMATION_STOP
};

const float ani_standby[] PROGMEM = {
  6,
  8, 2,
  8, 2,
  48, 1,
  48, 2,
  8, 1,
  8, 2,
  ANIMATION_REPEAT_KEYFRAME, 2
};

const float ani_working[] PROGMEM = {
  2,
  48, 0.75,
  255, 0.75,
  ANIMATION_REPEAT
};

const float ani_restart[] PROGMEM = {
  6,
  16, 0.125,
  16, 0.125,
  255, 0.25,
  32, 1,
  32, 1,
  255, 3,
  ANIMATION_STOP
};

const float ani_setup[] PROGMEM = {
  6,
  0, 0.1,
  255, 0.15,
  0, 0.15,
  255, 0.15,
  0, 0.15,
  255, 0.25,
  ANIMATION_STOP
};

const float ani_saveSetup[] PROGMEM = {
  5,
  0, 0.25,
  255, 0.25,
  0, 0.25,
  255, 0.25,
  0, 0.5,
  ANIMATION_STOP
};

const float ani_brightness[] PROGMEM = {
  2,
  0, 0.05,
  255, 0.15,
  ANIMATION_STOP
};

const float ani_smartMode[] PROGMEM = {
  4,
  0, 0.5,
  255, 0.75,
  0, 0.5,
  255, 0.75,
  ANIMATION_STOP
};

const float ani_passthroughMode[] PROGMEM = {
  8,
  0, 0.01,
  0, 0.49,
  255, 0.01,
  255, 0.49,
  0, 0.01,
  0, 0.49,
  255, 0.01,
  255, 0.49,
  ANIMATION_STOP
};

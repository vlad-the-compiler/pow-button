#pragma once

#include <Arduino.h>
#include <avr/pgmspace.h>

#include "animation_const.h"

// Animation config
#define ANIMATION_ROUTINE_HZ 100

// Animation derived config
#define ANIMATION_ROUTINE_US 1000000.0f / ANIMATION_ROUTINE_HZ

// Main loop variables
extern unsigned long lastAnimationTickUs;

// Animation controller
class Animated {
  private:
  const float *animation;
  const float *queued = NULL;
  float animationValue = 0;
  int animationKeyframe = 0;
  int animationSequenceKeyframes = 0;
  float animationTarget = 0;
  float animationStep = 0;
  float animationKeyframeDurationUs = 0;
  float animationKeyframeElapsedUs = 0;
  bool animationRunning = false;

  void onAnimationKeyframe(int keyframe);
  void onAnimationKeyframeComplete();

  public:
  void startAnimation(const float *animationPtr);
  void enqueueAnimation(const float *animationPtr);
  float runAnimation();
  bool animating();
  float getValue();
};

// Timekeeping routine
bool shouldRunAnimations();

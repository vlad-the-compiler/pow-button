#include "Animated.h"

// Main loop variables
unsigned long lastAnimationTickUs = 0;

// Animation controller methods
void Animated::onAnimationKeyframe(int keyframe) {
  if (keyframe < animationSequenceKeyframes) {
    animationTarget = pgm_read_float(animation + keyframe * 2);
    animationKeyframeDurationUs = pgm_read_float(animation + keyframe * 2 + 1) * 1000000;
    animationKeyframeElapsedUs = 0;
    animationStep = (animationTarget - animationValue) / (pgm_read_float(animation + keyframe * 2 + 1) * ANIMATION_ROUTINE_HZ);
    animationKeyframe = keyframe;
  }
}

void Animated::onAnimationKeyframeComplete() {
  if (animationKeyframe < animationSequenceKeyframes - 1) {
    animationKeyframe++;
    onAnimationKeyframe(animationKeyframe);
  } else {
    if (queued == NULL) {
      int endType = pgm_read_float(animation + animationKeyframe * 2 + 2);
      switch (endType) {
        case ANIMATION_STOP:
          animationRunning = false;
          break;
        case ANIMATION_REPEAT:
          onAnimationKeyframe(0);
          break;
        case ANIMATION_REPEAT_KEYFRAME:
          onAnimationKeyframe(pgm_read_float(animation + animationKeyframe * 2 + 3));
          break;
      }
    } else {
      startAnimation(queued);
      queued = NULL;
    }
  }
}

void Animated::startAnimation(const float *animationPtr) {
  animation = animationPtr;
  animationRunning = true;
  animationSequenceKeyframes = pgm_read_float(animation + 0);
  animation++;
  onAnimationKeyframe(0);
}

void Animated::enqueueAnimation(const float *animationPtr) {
  if (animationRunning) {
    queued = animationPtr;
  } else {
    startAnimation(animationPtr);
  }
}

float Animated::runAnimation() {
  if (animationRunning) {
    animationValue += animationStep;
    animationKeyframeElapsedUs += ANIMATION_ROUTINE_US;
    bool endKeyframe = false;
    if (animationKeyframeElapsedUs >= animationKeyframeDurationUs) {
      endKeyframe = true;
    }
    if (endKeyframe) {
      animationValue = animationTarget;
      onAnimationKeyframeComplete();
    }
  }
  return animationValue;
}

bool Animated::animating() {
  return animationRunning;
}

float Animated::getValue() {
  return animationValue;
}


// Timekeeping routine
bool shouldRunAnimations() {
  unsigned long currentUs = micros();
  if (currentUs - lastAnimationTickUs >= ANIMATION_ROUTINE_US) {
    lastAnimationTickUs = currentUs;
    return true;
  }
  return false;
}

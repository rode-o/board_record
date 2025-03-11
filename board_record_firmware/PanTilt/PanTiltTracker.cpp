#include "PanTiltTracker.h"
#include "Config.h"

PanTiltTracker::PanTiltTracker()
: panAngle(90.0f), 
  tiltAngle(90.0f), 
  prevErrorPan(0.0f), 
  prevErrorTilt(0.0f),
  servoPanAttached(false),
  servoTiltAttached(false)
{
  // Load PD gains from config
  kpPan   = KP_PAN;
  kdPan   = KD_PAN;
  kpTilt  = KP_TILT;
  kdTilt  = KD_TILT;
}

void PanTiltTracker::attach(int pinPan, int pinTilt) {
  servoPan.attach(pinPan);
  servoTilt.attach(pinTilt);

  servoPan.write(static_cast<int>(panAngle));
  servoTilt.write(static_cast<int>(tiltAngle));

  servoPanAttached  = true;
  servoTiltAttached = true;
}

void PanTiltTracker::update(int x, int y, int centerX, int centerY) {
  if (!servoPanAttached || !servoTiltAttached) {
    return;
  }

  // Calculate errors
  float errorX = static_cast<float>(x) - static_cast<float>(centerX);
  float errorY = static_cast<float>(y) - static_cast<float>(centerY);

  // Dead zone
  if (abs(errorX) < DEAD_ZONE) errorX = 0;
  if (abs(errorY) < DEAD_ZONE) errorY = 0;

  // PD for Pan
  float derivativePan = errorX - prevErrorPan;
  float outputPan = (kpPan * errorX) + (kdPan * derivativePan);
  prevErrorPan = errorX;

  // PD for Tilt
  float derivativeTilt = errorY - prevErrorTilt;
  float outputTilt = (kpTilt * errorY) + (kdTilt * derivativeTilt);
  prevErrorTilt = errorY;

  // Adjust angles
  // NOTE: Depending on your mechanical orientation, you may need
  // to invert these (e.g., panAngle += outputPan).
  panAngle  -= outputPan; 
  tiltAngle += outputTilt; 

  // Clamp
  panAngle  = clampValue(panAngle,  SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
  tiltAngle = clampValue(tiltAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

  // Write to servos
  servoPan.write(static_cast<int>(panAngle));
  servoTilt.write(static_cast<int>(tiltAngle));
}

float PanTiltTracker::clampValue(float val, float minVal, float maxVal) {
  if (val < minVal) return minVal;
  if (val > maxVal) return maxVal;
  return val;
}

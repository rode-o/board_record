#ifndef PAN_TILT_TRACKER_H
#define PAN_TILT_TRACKER_H

#include <Arduino.h>
#include <Servo.h>

// Forward declarations (if needed)
// None in this case

class PanTiltTracker {
public:
  // Constructor
  PanTiltTracker();

  // Attach the servos to their pins
  void attach(int pinPan, int pinTilt);

  // Update with new bounding box center; centerX, centerY is the desired center
  void update(int x, int y, int centerX, int centerY);

private:
  // Helper to clamp angles
  float clampValue(float val, float minVal, float maxVal);

  // PD gains (will load from config in .cpp)
  float kpPan, kdPan;
  float kpTilt, kdTilt;

  // Current servo angles
  float panAngle;
  float tiltAngle;

  // Previous errors (for derivative term)
  float prevErrorPan;
  float prevErrorTilt;

  // Servo objects
  Servo servoPan;
  Servo servoTilt;

  bool servoPanAttached;
  bool servoTiltAttached;
};

#endif // PAN_TILT_TRACKER_H

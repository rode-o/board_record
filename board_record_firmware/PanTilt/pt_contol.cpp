#include <Servo.h>

// === CONFIGURATION CONSTANTS ===
// Adjust these for your hardware and camera setup

// Servo pins
const int PIN_SERVO_PAN  = 9;
const int PIN_SERVO_TILT = 10;

// Camera resolution (width x height)
const int CAMERA_WIDTH  = 320;
const int CAMERA_HEIGHT = 240;

// Servo angle limits
const int SERVO_MIN_ANGLE = 0;
const int SERVO_MAX_ANGLE = 180;

// PD gains (tune these!)
float KpPan  = 0.07;  // Proportional gain for pan
float KdPan  = 0.10;  // Derivative gain for pan

float KpTilt = 0.07;  // Proportional gain for tilt
float KdTilt = 0.10;  // Derivative gain for tilt

// A small dead zone in pixels to prevent jitter around center
const int DEAD_ZONE = 3;

// How quickly the loop runs (ms). Lower = faster response, but also more load
const unsigned long LOOP_INTERVAL_MS = 50;  


// === HELPER FUNCTIONS ===

// In reality, you would replace this with code that reads 
// the bounding box center (x, y) from your ESP32 or other device.
void getBoundingBoxCenter(int &x, int &y) {
  // For demonstration, we'll just pick a random point. 
  // Replace with actual camera data logic.
  x = random(0, CAMERA_WIDTH);
  y = random(0, CAMERA_HEIGHT);
}

// Simple function to clamp a value within [minVal, maxVal]
float clampValue(float val, float minVal, float maxVal) {
  if (val < minVal) return minVal;
  if (val > maxVal) return maxVal;
  return val;
}


// === PD-BASED PAN-TILT TRACKER CLASS ===

class PanTiltTracker {
public:
  PanTiltTracker()
    : panAngle(90.0), tiltAngle(90.0), 
      prevErrorPan(0.0), prevErrorTilt(0.0), 
      servoPanAttached(false), servoTiltAttached(false) {}

  // Attach servo objects
  void attach(int pinPan, int pinTilt) {
    servoPan.attach(pinPan);
    servoTilt.attach(pinTilt);

    // Initialize to center
    servoPan.write((int)panAngle);
    servoTilt.write((int)tiltAngle);

    servoPanAttached = true;
    servoTiltAttached = true;
  }

  // Update the tracker with a new bounding box center
  // x, y in [0..CAMERA_WIDTH or CAMERA_HEIGHT]
  // centerX, centerY is the desired "center" (e.g., W/2, H/2)
  void update(int x, int y, int centerX, int centerY) {
    if (!servoPanAttached || !servoTiltAttached) return;

    // Compute error in X (for pan) and Y (for tilt)
    float errorX = (float)x - (float)centerX;
    float errorY = (float)y - (float)centerY;

    // If error is within dead zone, treat it as zero to avoid jitter
    if (abs(errorX) < DEAD_ZONE) errorX = 0;
    if (abs(errorY) < DEAD_ZONE) errorY = 0;

    // PAN: PD control
    float derivativePan = errorX - prevErrorPan;
    float outputPan = (KpPan * errorX) + (KdPan * derivativePan);
    prevErrorPan = errorX;

    // TILT: PD control
    float derivativeTilt = errorY - prevErrorTilt;
    float outputTilt = (KpTilt * errorY) + (KdTilt * derivativeTilt);
    prevErrorTilt = errorY;

    // Update angles
    // Note that a positive offset in x => object is to the right => 
    // we might want to increase panAngle to turn right, 
    // but this depends on your mechanical orientation. 
    // Adjust sign if needed.
    panAngle -= outputPan;   // might be += outputPan depending on servo orientation
    tiltAngle += outputTilt; // might be -= outputTilt depending on servo orientation

    // Clamp angles to servo limits
    panAngle  = clampValue(panAngle,  SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    tiltAngle = clampValue(tiltAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

    // Command servos
    servoPan.write((int)panAngle);
    servoTilt.write((int)tiltAngle);
  }

private:
  Servo servoPan;
  Servo servoTilt;

  float panAngle;
  float tiltAngle;

  float prevErrorPan;
  float prevErrorTilt;

  bool servoPanAttached;
  bool servoTiltAttached;
};


PanTiltTracker tracker;

// For simple timing
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0)); // If you have an unused analog pin for randomness

  // Attach the tracker to pins
  tracker.attach(PIN_SERVO_PAN, PIN_SERVO_TILT);

  lastUpdate = millis();
}

void loop() {
  unsigned long now = millis();

  // Run the tracking loop at a fixed interval
  if (now - lastUpdate >= LOOP_INTERVAL_MS) {
    lastUpdate = now;

    // 1) Get bounding box center from the camera
    int x, y;
    getBoundingBoxCenter(x, y);

    // 2) Desired center is half the width, half the height
    int centerX = CAMERA_WIDTH / 2;
    int centerY = CAMERA_HEIGHT / 2;

    // 3) Update the tracker (PD control for pan/tilt)
    tracker.update(x, y, centerX, centerY);

    // Debug print
    Serial.print("Target: (");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.println(")");
  }
}

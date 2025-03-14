# Xiao ESP32-S3 Powered Pan-Tilt Camera
#WIP
#THIS IS NOT A REAL README. THIS IS A TEMPORARY README GENERATED BY CHATGPT AND WILL NOT DESCRIBE THE PROJECT WITH 100% ACCURACY

A pan-tilt camera system built around a **Seeed Studio Xiao ESP32-S3** microcontroller that uses local object detection to track a person—ideal for recording lectures or presentations. This setup assists students (or anyone filming a speaker) to automatically follow the lecturer, capture notes, and record audio/video for later study or reference.

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Hardware Requirements](#hardware-requirements)
4. [Software Requirements](#software-requirements)
5. [Assembly and Wiring](#assembly-and-wiring)
6. [Setup and Configuration](#setup-and-configuration)
7. [Usage](#usage)
8. [Troubleshooting](#troubleshooting)
9. [License](#license)

---

## 1. Overview

This project uses an **ESP32-S3** variant from Seeed Studio (the Xiao ESP32-S3) to control a pan-tilt camera mount. It leverages onboard or external hardware-accelerated machine learning (ML) capabilities for local person detection.

### Key Goals:

- **Automatic Person Tracking**: The camera follows the teacher or subject in frame by analyzing the live video feed for person detection.
- **Low Latency**: Running object detection locally on the Xiao ESP32-S3 reduces reliance on network connectivity.
- **Classroom & Presentation Use**: Designed to simplify capturing lecture notes or demonstrations, enabling students or teachers to focus on teaching/learning instead of camera operation.
- **Compact Form Factor**: The Xiao ESP32-S3 is small enough to integrate neatly into a pan-tilt mechanism.

---

## 2. Features

- **Local Object Detection**: Uses machine learning models capable of identifying people in real time without requiring cloud processing.
- **Smooth Pan-Tilt**: Controls servo motors for horizontal (pan) and vertical (tilt) movement.
- **Lightweight and Portable**: The Xiao ESP32-S3 is small and integrates Wi-Fi/Bluetooth for optional connectivity.
- **Configurable**: Adjust detection thresholds, servo speed, or bounding box triggers.
- **Recording Setup**: Integrates with a small camera module to record or stream video.

---

## 3. Hardware Requirements

1. **Seeed Studio Xiao ESP32-S3**  
2. **Pan-Tilt Mechanism**: Typically consists of two micro servos (one for pan, one for tilt) and a bracket assembly.  
3. **Camera Module**:  
   - For simplest integration, consider a small camera such as the OV2640/OV5640 or any camera module supported by the ESP32-S3.  
4. **Power Supply**:  
   - A stable 5V supply for the servos (depending on servo specs).  
   - USB 5V or regulated supply for the Xiao ESP32-S3.  
5. **Servo Driver or Direct Pin Control**:  
   - Servos can be driven directly from the Xiao ESP32-S3 pins if the current draw is within specs. Often, a separate driver or external power line is recommended to avoid noise.  
6. **Optional Accessories**:  
   - Mounting hardware, additional sensors, or microphones for audio input.

---

## 4. Software Requirements

1. **Arduino IDE** or **PlatformIO**  
   - With the [Seeed Studio Xiao ESP32-S3 board support package](https://docs.platformio.org/en/latest/boards/espressif32/esp32-s3-devkitc-1.html) or via the Arduino Board Manager.  
2. **ESP-IDF** (if you prefer native development)  
3. **Library Dependencies**:
   - [Servo library or equivalent for ESP32-S3](https://github.com/RoboticsBrno/ServoESP32)  
   - [TinyML or TensorFlow Lite for Microcontrollers (optional)](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro) – for local object detection.  
   - [Camera driver library (depends on your camera choice)](https://github.com/espressif/esp32-camera).  

---

## 5. Assembly and Wiring

1. **Mount Servos** on the pan-tilt bracket.  
2. **Install Camera** on the tilt mechanism so the lens faces forward.  
3. **Connect Servos** to the Xiao ESP32-S3:  
   - **5V** (or 4.8–6 V, as recommended by the servo) to the servo’s power rail.  
   - **GND** from the Xiao to the servo’s ground.  
   - **Signal** pins from the Xiao digital pins to servo signal pins (for example, `GPIO14` for pan and `GPIO15` for tilt).  
4. **Connect Camera** to the Xiao ESP32-S3 camera pins (if using official modules, ensure the correct pin mapping).  
5. **Separate Power** (recommended):  
   - Servos can draw significant current. It’s best to provide a separate 5 V supply for the servos, sharing a common GND with the Xiao.

---

## 6. Setup and Configuration

1. **Install Board Support**  
   - In the Arduino IDE, go to **Tools > Board > Boards Manager** and search for “ESP32-S3” or “Seeed Xiao ESP32-S3,” then install.  
2. **Install Libraries**  
   - Add any required libraries for servos, camera, and ML detection.  
3. **Configure Pins**  
   - In your code, confirm the correct GPIO numbers for pan/tilt and camera signals.  
4. **Compile & Upload**  
   - Select the Xiao ESP32-S3 as the target board.  
   - Click **Verify** then **Upload**.  
5. **Model Placement** (if using local ML)  
   - Ensure your TensorFlow Lite model or similar is placed in the correct project folder.  
   - The code should reference the model array or file path.

---

## 7. Usage

1. **Power On**  
   - Plug in the Xiao ESP32-S3 via USB or attach an external power supply.  
2. **Initialize**  
   - The system will perform a brief self-test; servos might rotate to initial positions.  
3. **Person Detection**  
   - The onboard ML model or detection algorithm starts analyzing the video feed.  
4. **Auto-Tracking**  
   - When a person is detected in the frame, the pan-tilt mechanism adjusts to keep them centered.  
5. **Recording / Output**  
   - Depending on your implementation:
     - A microSD card or flash to store the video.  
     - Wireless streaming over Wi-Fi (if configured).  
6. **Adjust Settings**  
   - Use serial commands or a web interface (if added) to change detection thresholds, servo speed, or logging options.

---

## 8. Troubleshooting

- **Servo Jitter or Twitching**  
  - Use an external power supply for the servos and make sure you have a common ground.  
- **No Camera Output**  
  - Check pin mappings, confirm the camera is supported by the chosen driver, and test with example sketches.  
- **No Person Detection**  
  - Validate that the ML model is loading correctly. If you’re using TensorFlow Lite for Microcontrollers, ensure your model is small enough for available memory.  
- **System Reset or Freezing**  
  - Check power consumption. The servos might cause voltage drops if underpowered. Use a separate 5 V supply for the servos.

---

## 9. License

This project is released under the [MIT License](LICENSE) – feel free to modify and distribute for personal or commercial use.

---

### Additional Notes

- **Future Enhancements**:
  - Improved ML models for multi-person tracking or gesture recognition.
  - Integrating with a microphone array for audio tracking.
  - Web-based configuration UI for easy control and setup.

- **Contributing**:  
  Contributions, ideas, and feature requests are welcome! Feel free to open an issue or pull request if you have suggestions or improvements.

**Enjoy your automated pan-tilt camera system!**

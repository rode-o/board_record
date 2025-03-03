#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// You can use any UUIDs; these are just examples.
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println(">> A device just connected to this BLE server.");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println(">> A device just disconnected.");
    // Restart advertising so others can discover again
    pServer->startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server...");

  // 1) Initialize the BLE device
  BLEDevice::init("MyESP32S3");  // This name appears when you scan for devices
  
  // 2) Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 3) Create a BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 4) Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ   |
      BLECharacteristic::PROPERTY_WRITE  |
      BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_INDICATE
  );

  // 5) Add a Descriptor (optional, commonly used for notifications)
  pCharacteristic->addDescriptor(new BLE2902());

  // 6) Start the service
  pService->start();

  // 7) Start advertising so the ESP32-S3 is discoverable
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("BLE server is now advertising.");
}

void loop() {
  // In this simple example, nothing happens in loop()
  // The device is advertising and waiting for connections.
}

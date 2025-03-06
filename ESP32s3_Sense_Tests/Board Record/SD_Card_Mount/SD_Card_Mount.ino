//Cant get the sd to mount

#include <SPI.h>
#include <SD.h>

#define SD_CS    10  // Use default XIAO ESP32S3 CS pin
#define SD_CLK   7   // Default SCK
#define SD_MOSI  6   // Default MOSI
#define SD_MISO  8   // Default MISO

SPIClass hspi = SPIClass(HSPI);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("📝 Initializing SD Card...");

    pinMode(SD_CS, OUTPUT);
    hspi.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS, hspi, 1000000)) {  // Try 1MHz if failing
        Serial.println("❌ SD Card Mount Failed!");
        return;
    }

    Serial.println("✅ SD Card Mounted Successfully!");
}

void loop() {
    // Nothing needed here
}

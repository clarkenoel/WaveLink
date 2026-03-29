// Gesture Controller Firmware for Pico 2 WH

#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <EdgeImpulse.h>

MPU6050 mpu;
WiFiUDP udp;

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// UDP settings
const char* udpAddress = "192.168.1.100"; // Change to your target IP
const int udpPort = 1234;

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    mpu.initialize();
    if (!mpu.testConnection()) {
        Serial.println("MPU6050 Connection Failed");
    }
}

void loop() {
    // Read accelerometer and gyroscope data
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Edge Impulse inference (pseudo-code)
    // float inferenceResult = runInference(ax, ay, az, gx, gy, gz);

    // Send data via UDP
    udp.beginPacket(udpAddress, udpPort);
    // udp.write(inferenceResult);
    udp.endPacket();

    delay(100);
}
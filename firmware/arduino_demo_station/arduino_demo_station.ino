// ============================================================
// ARDUINO UNO R4 WIFI - DEMO STATION RECEIVER & CONTROLLER
// ============================================================
//
// This firmware:
//   1. Acts as a WiFi Access Point (SSID: WaveLinkNet)
//   2. Listens for UDP packets from the Pico gesture controller
//   3. Decodes gesture packets (index, confidence, name)
//   4. Controls physical actuators (relay, servo, motor, LCD)
//   5. Displays gesture info on 16×2 LCD
//   6. Provides visual/audio feedback
//
// ============================================================
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
// ============================================================
// NETWORK CONFIGURATION
// ============================================================
const char* ssid = "WaveLinkNet";
const char* password = "Trailblazers1234";
const int udpPort = 4210;
// ============================================================
// PIN CONFIGURATION
// ============================================================
#define RELAY_PIN        5   // Relay for lamp (Flick Down)
#define SERVO_PIN        6   // Servo for door (Flick Up)
#define MOTOR_PIN_1      7   // Motor direction 1 (Turn Right)
#define MOTOR_PIN_2      8   // Motor direction 2 (Turn Left)
#define LCD_SDA          20  // I2C SDA (LCD)
#define LCD_SCL          21  // I2C SCL (LCD)
#define LED_PIN          13  // Status LED
#define BUZZER_PIN       9   // Audio feedback buzzer
// ============================================================
// PACKET CONSTANTS
// ============================================================
#define PACKET_SIZE 18
#define CMD_HEARTBEAT 255
// ============================================================
// GLOBAL OBJECTS
// ============================================================
WiFiServer wifiServer(80);
WiFiUDP udp;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16x2 display
Servo doorServo;
// State tracking
unsigned long lastGestureTime = 0;
unsigned long lastHeartbeatTime = 0;
int relayState = LOW;
int servoPos = 0;
// ============================================================
// SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    delay(1000);
    Serial.println("\n========================================");
    Serial.println("Arduino Demo Station v1.0");
    Serial.println("========================================\n");
    // Initialize pins
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(MOTOR_PIN_1, OUTPUT);
    pinMode(MOTOR_PIN_2, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(MOTOR_PIN_1, LOW);
    digitalWrite(MOTOR_PIN_2, LOW);
    digitalWrite(LED_PIN, LOW);
    // Initialize LCD
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Wavelink Demo");
    lcd.setCursor(0, 1);
    lcd.print("Starting...");
    Serial.println("LCD initialized");
    // Initialize servo
    doorServo.attach(SERVO_PIN);
    doorServo.write(0);  // Start at 0 degrees
    Serial.println("Servo initialized");
    // Setup WiFi as Access Point
    setupWiFi();
    // Initialize UDP
    udp.begin(udpPort);
    Serial.print("UDP listening on port ");
    Serial.println(udpPort);
    Serial.println("========================================");
    Serial.println("Ready to receive gestures from Pico!");
    Serial.println("========================================\n");
    // Display ready message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wavelink Ready");
    lcd.setCursor(0, 1);
    lcd.print("Waiting...");
}
// ============================================================
// MAIN LOOP
// ============================================================
void loop() {
    // Check for UDP packets
    int packetSize = udp.parsePacket();
    if (packetSize == PACKET_SIZE) {
        // Receive the packet
        uint8_t packet[PACKET_SIZE];
        udp.read(packet, PACKET_SIZE);
        // Decode packet
        uint8_t gestureIndex = packet[0];
        uint8_t confidence = packet[1];
        char gestureName[16];
        strncpy(gestureName, (char*)&packet[2], 15);
        gestureName[15] = '\0';
        Serial.print(">> Received: ");
        Serial.print(gestureName);
        Serial.print(" (");
        Serial.print(confidence);
        Serial.print("%) index=");
        Serial.println(gestureIndex);
        // Update heartbeat
        lastHeartbeatTime = millis();
        // Handle heartbeat packets (do nothing, just update time)
        if (gestureIndex == CMD_HEARTBEAT) {
            Serial.println("   [HEARTBEAT received]");
            digitalWrite(LED_PIN, HIGH);
            delay(50);
            digitalWrite(LED_PIN, LOW);
            return;
        }
        // Handle gesture commands
        handleGesture(gestureIndex, confidence, gestureName);
        lastGestureTime = millis();
    }
    // Check WiFi connection periodically
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 5000) {
        lastCheck = millis();
        if (WiFi.status() == WL_AP_LISTENING) {
            Serial.print(".");
        }
    }
}
// ============================================================
// GESTURE HANDLER
// ============================================================
void handleGesture(uint8_t index, uint8_t confidence, char* name) {
    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(name);
    lcd.setCursor(0, 1);
    lcd.print("Conf: ");
    lcd.print(confidence);
    lcd.print("%);");
    // Provide feedback
    tone(BUZZER_PIN, 1000, 100);  // 1000Hz for 100ms
    delay(50);
    // Execute gesture-specific action
    switch (index) {
        case 1:  // Flick Down
            executeFlickDown();
            break;
        case 2:  // Flick Left
            executeFlickLeft();
            break;
        case 3:  // Flick Right
            executeFlickRight();
            break;
        case 4:  // Flick Up
            executeFlickUp();
            break;
        case 0:  // Idle
            // Do nothing
            break;
        default:
            Serial.print("Unknown gesture index: ");
            Serial.println(index);
    }
    Serial.print("   Action executed: ");
    Serial.println(name);
}
// ============================================================
// GESTURE ACTIONS
// ============================================================
void executeFlickDown() {
    // Toggle relay (lamp on/off)
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState);
    tone(BUZZER_PIN, 1500, 150);
    delay(50);
    tone(BUZZER_PIN, 1000, 150);
    Serial.print("   Lamp: ");
    Serial.println(relayState ? "ON" : "OFF");
}
void executeFlickUp() {
    // Raise servo door
    for (int i = 0; i <= 90; i += 10) {
        doorServo.write(i);
        delay(30);
    }
    servoPos = 90;
    tone(BUZZER_PIN, 1200, 100);
    delay(100);
    tone(BUZZER_PIN, 1400, 100);
    Serial.println("   Door: OPEN");
    delay(2000);  // Keep open for 2 seconds
    // Close servo door
    for (int i = 90; i >= 0; i -= 10) {
        doorServo.write(i);
        delay(30);
    }
    servoPos = 0;
    Serial.println("   Door: CLOSED");
}
void executeFlickLeft() {
    // Spin motor counter-clockwise (Turn Left)
    digitalWrite(MOTOR_PIN_1, LOW);
    digitalWrite(MOTOR_PIN_2, HIGH);
    tone(BUZZER_PIN, 800, 200);
    delay(100);
    tone(BUZZER_PIN, 1000, 200);
    Serial.println("   Motor: COUNTER-CLOCKWISE");
    delay(1500);  // Spin for 1.5 seconds
    // Stop motor
    digitalWrite(MOTOR_PIN_1, LOW);
    digitalWrite(MOTOR_PIN_2, LOW);
    Serial.println("   Motor: STOPPED");
}
void executeFlickRight() {
    // Spin motor clockwise (Turn Right)
    digitalWrite(MOTOR_PIN_1, HIGH);
    digitalWrite(MOTOR_PIN_2, LOW);
    tone(BUZZER_PIN, 1200, 200);
    delay(100);
    tone(BUZZER_PIN, 1400, 200);
    Serial.println("   Motor: CLOCKWISE");
    delay(1500);  // Spin for 1.5 seconds
    // Stop motor
    digitalWrite(MOTOR_PIN_1, LOW);
    digitalWrite(MOTOR_PIN_2, LOW);
    Serial.println("   Motor: STOPPED");
}
// ============================================================
// WIFI SETUP (ACCESS POINT MODE)
// ============================================================
void setupWiFi() {
    Serial.print("Setting up WiFi Access Point: ");
    Serial.println(ssid);
    // Start as access point
    WiFi.beginAP(ssid, password);
    // Wait for AP to start
    int attempts = 0;
    while (WiFi.status() != WL_AP_LISTENING && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_AP_LISTENING) {
        Serial.println("\nWiFi AP started successfully!");
        Serial.print("  SSID: ");
        Serial.println(ssid);
        Serial.print("  Password: ");
        Serial.println(password);
        Serial.print("  IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Failed to start WiFi AP!");
    }
}
// ============================================================
// HELPER FUNCTIONS
// ============================================================
void beep(int frequency, int duration) {
    tone(BUZZER_PIN, frequency, duration);
}
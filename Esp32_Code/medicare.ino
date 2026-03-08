#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid     = "USERNAME";
const char* password = "PASSWORD";

const char* serverURL = "https://medicare-backend.com/dosage"; // use you URL based on your server if you dump this into esp32.

#define IR_SENSOR_PIN D2

bool notificationSent = false;
unsigned long lastCheckTime = 0;
const long checkInterval = 5000;

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_PIN, INPUT);
  Serial.println("\n🚀 MediCare ESP8266 Starting...");
  WiFi.begin(ssid, password);
  Serial.print("📶 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📍 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("\n📋 Commands:");
  Serial.println("  Type '1' → Dosage Completed");
  Serial.println("  Type '0' → Dosage Missed");
  Serial.println("  Type 'r' → Reset for next dosage");
  Serial.println("\n⏳ Waiting for input...\n");
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '1' && !notificationSent) {
      Serial.println("💊 Pill TAKEN — Sending completed notification...");
      sendDosageStatus(1);
      notificationSent = true;
    } else if (input == '0' && !notificationSent) {
      Serial.println("⚠️  Pill MISSED — Sending missed notification...");
      sendDosageStatus(0);
      notificationSent = true;
    } else if (input == 'r') {
      Serial.println("🔄 Reset! Ready for next dosage check.");
      notificationSent = false;
    }
  }
}

void sendDosageStatus(int status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected!");
    return;
  }
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, serverURL);
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"status\":" + String(status) + ",\"device_id\":\"esp8266\"}";
  Serial.print("📡 Sending to Render: ");
  Serial.println(payload);
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("✅ Server response (");
    Serial.print(httpResponseCode);
    Serial.print("): ");
    Serial.println(response);
  } else {
    Serial.print("❌ Error sending: ");
    Serial.println(http.errorToString(httpResponseCode));
  }
  http.end();
}

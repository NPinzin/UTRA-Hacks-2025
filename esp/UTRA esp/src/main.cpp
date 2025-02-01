#include <WiFi.h>

// Wi-Fi credentials
const char* ssid = "Alinas iPhone";       // Replace with your Wi-Fi SSID
const char* password = "gojeta98";        // Replace with your Wi-Fi password

void setup() {
  // Start Serial communication for debugging
  Serial.begin(115200);  // Baud rate must match the Serial Monitor setting
  delay(1000);           // Wait for the Serial Monitor to start

  // Print initial Wi-Fi status
  Serial.println("Initializing Wi-Fi...");
  Serial.print("Wi-Fi Status: ");
  Serial.println(WiFi.status());

  // Attempt to connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  // Wait until the ESP32 connects to the Wi-Fi or 20 attempts
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  Serial.println();  // Newline after attempts

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  // Print the IP address assigned to the ESP32
  } else {
    Serial.println("Wi-Fi connection failed!");
    Serial.print("Error Code: ");
    Serial.println(WiFi.status());  // Print the failure reason
  }
}

void loop() {
  // Empty loop since we're only connecting to Wi-Fi in setup
}
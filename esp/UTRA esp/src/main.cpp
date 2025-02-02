#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Wi-Fi credentials
const char* ssid = "Alinas iPhone";
const char* password = "gojeta98";

// Your backend URL (adjust to your server’s LAN IP & port)
const char* backendUrl = "http://172.20.10.2:5001/api/posts/all-posts";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed!");
  }
}

void fetchPosts() {
  HTTPClient http;
  http.begin(backendUrl);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Payload received:");
      Serial.println(payload);

      // We'll parse the "data" array
      const size_t capacity = 10240;  // ~10KB buffer, adjust if needed
      DynamicJsonDocument doc(capacity);

      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        http.end();
        return;
      }

      // doc["data"] should be an array of posts
      JsonArray dataArray = doc["data"].as<JsonArray>();
      if (!dataArray.isNull()) {
        for (JsonObject post : dataArray) {
          const char* description = post["description"];
          if (description) {
            // Now parse the JSON in 'description'
            DynamicJsonDocument descDoc(1024);
            DeserializationError descErr =
                deserializeJson(descDoc, description);
            if (!descErr) {
              // Example fields inside 'description'
              const char* seatId = descDoc["seatId"];
              JsonObject loc = descDoc["location"];
              if (!loc.isNull()) {
                float x = loc["x"];
                float y = loc["y"];

                Serial.print("Seat: ");
                Serial.print(seatId ? seatId : "unknown");
                Serial.print(" => x=");
                Serial.print(x);
                Serial.print(", y=");
                Serial.println(y);

                // Do something with x,y
                // e.g., store them, send to motors, etc.
              }
            } else {
              Serial.print("Error parsing 'description': ");
              Serial.println(descErr.c_str());
            }
          }
        }
      } else {
        Serial.println("No 'data' array found in response.");
      }
    } else {
      Serial.print("HTTP GET failed, code: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.print("HTTP GET error: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    fetchPosts();
  } else {
    Serial.println("Not connected to WiFi");
  }

  // Fetch every 10 seconds
  delay(10000);
}

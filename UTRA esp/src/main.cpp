#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <vector>
#include <algorithm>
#include <time.h>

//---------------------------------------------
// Configuration for WiFi and Backend URL
//---------------------------------------------
const char* ssid = "Alinas iPhone";
const char* password = "gojeta98";
const char* backendUrl = "http://172.20.10.2:5001/api/posts/all-posts";

//---------------------------------------------
// Helper: Format registration time as "H:MM AM/PM"
//---------------------------------------------
String formatRegistrationTime(time_t regTime) {
  char buf[16];
  struct tm timeinfo;
  localtime_r(&regTime, &timeinfo);
  // Format with a possible leading zero, which we then remove if present.
  strftime(buf, sizeof(buf), "%I:%M %p", &timeinfo);
  if(buf[0] == '0'){
    return String(buf + 1);
  }
  return String(buf);
}

//---------------------------------------------
// Struct to hold data for an active patient (one currently in a seat)
//---------------------------------------------
struct ActivePatient {
  String userId;
  String firstName;
  String seatId;           // Current seat ID
  float x;                 // x coordinate of the seat
  float y;                 // y coordinate of the seat
  time_t registrationTime; // Time when the patient was registered
  bool visited;            // Flag to indicate if the robot has visited the patient
};

// Global active queue containing only patients with a seat.
// The queue is kept ordered by registrationTime (earliest first).
std::vector<ActivePatient> activeQueue;

//---------------------------------------------
// Process an individual post from the server.
// For each post:
//   - If a seat is assigned, add or update the patient record.
//   - If the seatId is empty, remove the patient (they’re no longer seated).
//
// Update logic now “forgets” any previous visit information if the current
// record is from a new day or if the seat has changed.
//---------------------------------------------
void processPost(JsonObject post) {
  // Get the userId (unique for each patient)
  const char* userIdC = post["userId"];
  String userId = userIdC ? String(userIdC) : "";

  // The "description" field is a JSON string with patient details.
  const char* description = post["description"];
  if (!description) {
    return;
  }
  
  DynamicJsonDocument descDoc(1024);
  DeserializationError descErr = deserializeJson(descDoc, description);
  if (descErr) {
    Serial.print("Error parsing 'description': ");
    Serial.println(descErr.c_str());
    return;
  }

  // Get patient name (if available)
  String firstName = "";
  if (descDoc.containsKey("firstName")) {
    firstName = String((const char*)descDoc["firstName"]);
  }
  
  // Get seatId. If null or missing, treat it as an empty string.
  String seatId = "";
  if (descDoc.containsKey("seatId") && !descDoc["seatId"].isNull()) {
    seatId = String((const char*)descDoc["seatId"]);
  }

  // Get the location coordinates.
  JsonObject loc = descDoc["location"];
  float x = 0.0, y = 0.0;
  if (!loc.isNull()) {
    x = loc["x"] | 0.0;
    y = loc["y"] | 0.0;
  }

  // Current time
  time_t now = time(NULL);
  struct tm now_tm;
  localtime_r(&now, &now_tm);

  // Look for this patient in the activeQueue.
  bool found = false;
  for (auto it = activeQueue.begin(); it != activeQueue.end(); ++it) {
    if (it->userId == userId) {
      found = true;
      // If the patient now has no seat, remove them.
      if (seatId == "") {
        Serial.print("Patient ");
        Serial.print(it->firstName);
        Serial.println(" left the chair. Removing from active queue.");
        activeQueue.erase(it);
      }
      else {
        // Determine the registration date (local day) for the stored record.
        struct tm reg_tm;
        localtime_r(&(it->registrationTime), &reg_tm);
        
        // If the seat has changed or the stored registration date is not today,
        // update the record with current information and reset the visited flag.
        if (it->seatId != seatId ||
            now_tm.tm_year != reg_tm.tm_year ||
            now_tm.tm_mon != reg_tm.tm_mon ||
            now_tm.tm_mday != reg_tm.tm_mday) {
          Serial.print("Updating patient ");
          Serial.print(firstName);
          Serial.print(" (UserId: ");
          Serial.print(userId);
          Serial.println(") as a new current entry.");
          it->seatId = seatId;
          it->registrationTime = now;
          it->visited = false;
        }
        // Otherwise, keep the current visited flag but update name and position.
        it->firstName = firstName;
        it->x = x;
        it->y = y;
      }
      break;  // Found the patient; break out of the loop.
    }
  }
  // If the patient is not found in the queue and they have a seat, add them.
  if (!found && seatId != "") {
    ActivePatient newPatient;
    newPatient.userId = userId;
    newPatient.firstName = firstName;
    newPatient.seatId = seatId;
    newPatient.x = x;
    newPatient.y = y;
    newPatient.registrationTime = now;
    newPatient.visited = false;
    activeQueue.push_back(newPatient);
    Serial.print("New patient registered: ");
    Serial.print(firstName);
    Serial.print(" (UserId: ");
    Serial.print(userId);
    Serial.print(") at seat ");
    Serial.println(seatId);
  }
}

//---------------------------------------------
// Fetch all posts from the backend, process each post,
// update the activeQueue, and then print the active queue
// in the desired format.
//---------------------------------------------
void fetchPosts() {
  HTTPClient http;
  http.begin(backendUrl);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("\n--- New Fetch ---");
      Serial.println("Payload received:");
      Serial.println(payload);

      // Deserialize the received JSON.
      const size_t capacity = 10240;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        http.end();
        return;
      }

      // Process each post in the "data" array.
      JsonArray dataArray = doc["data"].as<JsonArray>();
      if (!dataArray.isNull()) {
        for (JsonObject post : dataArray) {
          processPost(post);
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

  // Sort the activeQueue by registration time (earliest first).
  std::sort(activeQueue.begin(), activeQueue.end(), [](const ActivePatient &a, const ActivePatient &b) {
    return a.registrationTime < b.registrationTime;
  });

  // Print each active patient in the desired format.
  for (auto &patient : activeQueue) {
    Serial.print("Active Patients Queue: Patient: ");
    Serial.print(patient.firstName);
    Serial.print(", Seat: ");
    Serial.print(patient.seatId);
    Serial.print(", Position: (");
    Serial.print(patient.x, 2);
    Serial.print(", ");
    Serial.print(patient.y, 2);
    Serial.print("), Registered at: ");
    Serial.print(formatRegistrationTime(patient.registrationTime));
    Serial.print(" , Visited: ");
    Serial.println(patient.visited ? "Yes" : "No");
  }
}

//---------------------------------------------
// Helper: Guide the robot to the next unvisited patient.
// (Replace the Serial prints with your actual navigation code.)
//---------------------------------------------
void guideRobotToNextPatient() {
  // Ensure the activeQueue is sorted.
  std::sort(activeQueue.begin(), activeQueue.end(), [](const ActivePatient &a, const ActivePatient &b) {
    return a.registrationTime < b.registrationTime;
  });

  for (auto &patient : activeQueue) {
    if (!patient.visited) {
      Serial.print("Guiding robot to patient ");
      Serial.print(patient.firstName);
      Serial.print(" at seat ");
      Serial.print(patient.seatId);
      Serial.print(" located at (");
      Serial.print(patient.x, 2);
      Serial.print(", ");
      Serial.print(patient.y, 2);
      Serial.println(")");
      // Insert your robot navigation code here.
      return;
    }
  }
  Serial.println("No unvisited patient available for guidance.");
}

//---------------------------------------------
// Helper: Mark a patient as visited.
// (Call this function when the robot finishes interacting with a patient.)
//---------------------------------------------
void markPatientVisited(String userId) {
  for (auto &patient : activeQueue) {
    if (patient.userId == userId) {
      patient.visited = true;
      Serial.print("Patient ");
      Serial.print(patient.firstName);
      Serial.println(" has been marked as visited.");
      return;
    }
  }
  Serial.println("Patient with given userId not found.");
}

//---------------------------------------------
// Setup: Connect to WiFi and configure NTP for real-time clock
//---------------------------------------------
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

  // Configure NTP to obtain real-world time.
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  } else {
    char timeStr[16];
    strftime(timeStr, sizeof(timeStr), "%I:%M %p", &timeinfo);
    Serial.print("Current time: ");
    Serial.println(timeStr);
  }
}

//---------------------------------------------
// Main loop: Fetch posts and guide robot every 10 seconds
//---------------------------------------------
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    fetchPosts();
    guideRobotToNextPatient();
  } else {
    Serial.println("Not connected to WiFi");
  }
  
  // Wait 10 seconds before next fetch.
  delay(10000);

  // For demonstration purposes, you might simulate marking a patient as visited:
  // if (!activeQueue.empty() && !activeQueue[0].visited) {
  //   markPatientVisited(activeQueue[0].userId);
  // }
}
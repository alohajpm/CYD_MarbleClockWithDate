#include "home_assistant.h"
#include <vector>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
const char* ssid = "JN24"; // Replace with your Wi-Fi SSID
const char* password = "D3skt0pK1ng"; // Replace with your Wi-Fi password


bool HomeAssistant::connect(const std::string& url, const std::string& user, const std::string& password) {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected.");

  Serial.print("Connecting to Home Assistant at: ");
  Serial.println(url.c_str());

  HTTPClient http;
  http.begin(url.c_str());

  // Set up basic authentication
  String authHeader = "Basic ";
  authHeader += base64::encode(user + ":" + password);
  http.addHeader("Authorization", authHeader);

  // Attempt to connect
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    Serial.println("Successfully connected to Home Assistant.");
    http.end();
    return true;
  } else {
    Serial.print("Failed to connect to Home Assistant. HTTP error code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println("Payload: "+ payload);
    http.end();
    return false;
  }

}

std::vector<std::string> HomeAssistant::discoverLights(const std::string& url, const std::string& user, const std::string& password) {
  Serial.println("Discovering lights from Home Assistant...");
  std::vector<std::string> lights;

  HTTPClient http;
  String lightURL = url;
  lightURL += "/api/states"; // Endpoint to get all states
  http.begin(lightURL);

  // Set up basic authentication
  String authHeader = "Basic ";
  authHeader += base64::encode(user + ":" + password);
  http.addHeader("Authorization", authHeader);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    Serial.println("Successfully retrieved states.");
    String payload = http.getString();
    Serial.println("Payload: " + payload);

    DynamicJsonDocument doc(8192); // Increased buffer size
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      http.end();
      return lights;
    }

    for (JsonVariant v : doc.as<JsonArray>()) {
      if (v["entity_id"].as<String>().startsWith("light.")) {
        lights.push_back(v["entity_id"].as<String>().c_str());
      }
    }

  } else {
    Serial.print("Failed to retrieve states. HTTP error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return lights;
}

bool HomeAssistant::toggleLight(const std::string& url, const std::string& user, const std::string& password, const std::string& lightId, const std::string& lightState) {
  Serial.print("Toggling light with ID: " );
  Serial.println(lightId.c_str());
    HTTPClient http;
    String lightURL = url;
    lightURL += "/api/services/light/toggle"; // Endpoint to toggle light
    http.begin(lightURL);

    // Set up basic authentication
    String authHeader = "Basic ";
    authHeader += base64::encode(user + ":" + password);
    http.addHeader("Authorization", authHeader);
    http.addHeader("Content-Type", "application/json");

    // Construct JSON payload for toggling the light
    String payload;
    if (lightState == "on") {
        payload = "{\"entity_id\":\"" + lightId + "\",\"state\":\"off\"}";

    } else {
        payload = "{\"entity_id\":\"" + lightId + "\",\"state\":\"on\"}";

    }
      Serial.print("Payload: ");
        Serial.println(payload);
    // Change to POST
    http.end();
    http.begin(lightURL);
  http.addHeader("Authorization", authHeader);
  http.addHeader("Content-Type", "application/json");

    // Attempt to send POST request
      int httpResponseCode = http.POST(payload);

    if (httpResponseCode == 200) {
      Serial.println("Successfully toggled light.");
      String response = http.getString();
        Serial.println("Response: "+ response);
      http.end();
      return true;
    } else {
      Serial.print("Failed to toggle light. HTTP error code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
        Serial.println("Response: "+ response);
      http.end();
      return false;
    }

}

std::string HomeAssistant::getLightState(const std::string& lightId) {
    Serial.println("Getting state of light with ID: " + lightId.c_str());
    return "";
}

std::string HomeAssistant::getLightState(const std::string& url, const std::string& user, const std::string& password, const std::string& lightId) {
    Serial.print("Getting state of light with ID: ");
    Serial.println(lightId.c_str());
      HTTPClient http;
      String stateURL = url;
    stateURL += "/api/states/";
      stateURL += lightId;
      http.begin(stateURL);

      // Set up basic authentication
      String authHeader = "Basic ";
      authHeader += base64::encode(user + ":" + password);
      http.addHeader("Authorization", authHeader);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.GET();

      if (httpResponseCode == 200) {
        Serial.println("Successfully retrieved light state.");
        String payload = http.getString();
          Serial.println("Payload: "+ payload);

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        String state = doc["state"].as<String>();
        http.end();
        return state;
      } else {
        Serial.print("Failed to retrieve light state. HTTP error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return "off";
      }
}

float HomeAssistant::getTemperature(const std::string& url, const std::string& user, const std::string& password, const std::string& entityId) {
  Serial.print("Getting temperature for entity: ");
  Serial.println(entityId.c_str());

  HTTPClient http;
  String tempURL = url;
  tempURL += "/api/states/";
  tempURL += entityId;
  http.begin(tempURL);

  // Set up basic authentication
  String authHeader = "Basic ";
  authHeader += base64::encode(user + ":" + password);
  http.addHeader("Authorization", authHeader);
  http.addHeader("Content-Type", "application/json");

  // Attempt to connect
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    Serial.println("Successfully got temperature.");
    String payload = http.getString();
    Serial.println("Payload: "+ payload);

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      float temp = doc["state"].as<float>();
    
    http.end();
    return temp;
  } else {
    Serial.print("Failed to get temperature. HTTP error code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println("Payload: "+ payload);
    http.end();
    return 0.0f;
  }
}


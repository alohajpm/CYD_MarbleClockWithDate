#include "home_assistant.h"
#include <iostream>
#include <vector>
#include <Arduino.h>
#include <WiFi.h>

bool HomeAssistant::connect(const std::string& url, const std::string& user, const std::string& password) {
    std::cout << "Connecting to Home Assistant with URL: " << url << ", User: " << user << ", Password: " << password << std::endl;
    return true;
}

std::vector<std::string> HomeAssistant::discoverLights() {
    std::cout << "Discovering lights from Home Assistant..." << std::endl;
    std::vector<std::string> lights;
    lights.push_back("light.example1");
    return lights;
}

bool HomeAssistant::toggleLight(const std::string& lightId) {
    std::cout << "Toggling light with ID: " << lightId << std::endl;
    return true;
}

std::string HomeAssistant::getLightState(const std::string& lightId) {
    std::cout << "Getting state of light with ID: " << lightId << std::endl;
    return "";
}

float HomeAssistant::getTemperature(const std::string& zipCode) {
   std::cout << "Getting temperature for zip: " << zipCode << std::endl;
    return 0.0f;

}

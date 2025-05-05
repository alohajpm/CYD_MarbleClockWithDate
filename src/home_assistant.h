#ifndef HOME_ASSISTANT_H
#define HOME_ASSISTANT_H

#include <string>
#include <vector>
#include <ArduinoJson.h>

class HomeAssistant {
public:
  HomeAssistant(const std::string& url, const std::string& user, const std::string& password);

  bool connect(const std::string& url, const std::string& user, const std::string& password);
  std::vector<std::string> discoverLights(const std::string& url, const std::string& user, const std::string& password_str);
  bool toggleLight(const std::string& url, const std::string& user, const std::string& password_str, const std::string& lightId, const std::string& lightState);
  std::string getLightState(const std::string& lightId);
  float getTemperature(const std::string& url, const std::string& user, const std::string& password_str, const std::string& entityId);

private:
    std::string url;
    std::string user;
    std::string password;



};

#endif
#ifndef HOME_ASSISTANT_H
#define HOME_ASSISTANT_H

#include <string>
#include <vector>

class HomeAssistant {
public:
  bool connect(const std::string& url, const std::string& user, const std::string& password) ;
  std::vector<std::string> discoverLights();
  bool toggleLight(const std::string& lightId);
  std::string getLightState(const std::string& lightId);
  float getTemperature(const std::string& zipCode);

private:
    // Private members for Home Assistant connection and data
    // ...
};

#endif
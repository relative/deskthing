#include "ctmgr.h"
#include "../common.h"

CarThingManager* gCarThingManager;

void CarThingManager::addDevice(std::shared_ptr<CarThing> ct) {
  std::lock_guard guard(mutDevice);
  devices.push_back(std::move(ct));
}

void CarThingManager::removeDeviceBySocket(SOCKET s) {
  std::shared_ptr<CarThing> tmp;

  std::lock_guard guard(mutDevice);

  auto it = std::find_if(devices.begin(), devices.end(), [&](auto&& ct) {
    return ct->sock == s;
  });
  if (it == devices.end()) return;
  
  tmp = std::move(*it);
  devices.erase(it);
}

void CarThingManager::publish(std::string& topic, nlohmann::json& details) {
  std::lock_guard guard(mutDevice);
  auto id = ++publicationId;
  for (const auto& dev : devices) {
    dev->publish(topic, details, id);
  }
}

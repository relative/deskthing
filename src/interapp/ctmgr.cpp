#include "ctmgr.h"
#include "../common.h"
#include "ia.h"

CarThingManager* gCarThingManager;

void CarThingManager::addDevice(std::shared_ptr<CarThing> ct) {
  std::lock_guard guard(mutDevice);
  ct->devId = ++ctrDevId;
  devices.push_back(std::move(ct));
}

void CarThingManager::removeDeviceById(int devId) {
  std::shared_ptr<CarThing> tmp;

  std::lock_guard guard(mutDevice);

  auto it = std::find_if(devices.begin(), devices.end(), [&](auto&& ct) {
    return ct->devId == devId;
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

void CarThingManager::reply(int devId, int reqId, nlohmann::json& details, nlohmann::json& args, nlohmann::json& argsKw) {
  std::lock_guard guard(mutDevice);
  for (const auto& dev : devices) {
    if (dev->devId == devId) {
      dev->send({
        MessageCode::RESULT,
        reqId,
        details,
        args,
        argsKw
      });
      break;
    }
  }
}
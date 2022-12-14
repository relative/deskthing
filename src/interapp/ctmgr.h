#ifndef INTERAPP_CTMGR_H
#define INTERAPP_CTMGR_H

#include "carthing.h"
#include <mutex>

class CarThingManager {
  public:
    CarThingManager() {};
    void addDevice(std::shared_ptr<CarThing> ct);
    void removeDeviceById(int devId);
    void publish(std::string& topic, nlohmann::json& details);
    void reply(int devId, int reqId, nlohmann::json& details, nlohmann::json& args, nlohmann::json& argsKw);
  private:
    std::vector<std::shared_ptr<CarThing>> devices;
    std::mutex mutDevice;
    int publicationId = 0;

    int ctrDevId = 0;
};

extern CarThingManager* gCarThingManager;

#endif /* INTERAPP_CTMGR_H */

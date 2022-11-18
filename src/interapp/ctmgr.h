#ifndef INTERAPP_CTMGR_H
#define INTERAPP_CTMGR_H

#include "carthing.h"
#include <mutex>

class CarThingManager {
  public:
    CarThingManager() {};
    void addDevice(std::shared_ptr<CarThing> ct);
    void removeDeviceBySocket(SOCKET s);
  private:
    std::vector<std::shared_ptr<CarThing>> devices;
    std::mutex mutDevice;
};

extern CarThingManager* gCarThingManager;

#endif /* INTERAPP_CTMGR_H */

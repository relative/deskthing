#ifndef INTERAPP_CARTHING_H
#define INTERAPP_CARTHING_H

#include <nlohmann/json.hpp>
#include "../platform/platform.h"
#include <thread>
#include <mutex>

#define MAX_BUF_LEN 1024

class CarThing {
  public:
    static std::shared_ptr<CarThing> create(SOCKET s);
    void poll();
    int send(nlohmann::json& j);
    int send(const nlohmann::json& j);
    SOCKET sock;
    ~CarThing();

    void subscribe(int reqId, std::string topic);
    void unsubscribe(int reqId, int subId);
    int publish(std::string& topic, nlohmann::json& details, int publicationId);

    int devId = 0;
  private:
    CarThing(SOCKET s) : sock(s) {};

    int processRPC(int reqId, std::string& proc, nlohmann::json& args, nlohmann::json& argsKw);
    void processMessage(nlohmann::json& j);

    std::thread thrd;
    bool authed = false;
    bool dead = false;
    int wampSessionId;

    int subscriptionId = 0;
    std::map<std::string, int> subscriptions;

    std::mutex mutSub;
    std::mutex mutSend;
};

#endif /* INTERAPP_CARTHING_H */

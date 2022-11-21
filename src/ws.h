#ifndef WS_H
#define WS_H

#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>

#define WS_PORT 36308
#define WS_TOPIC "update"

struct PerSocketData { };

namespace websocket {
  extern uWS::TemplatedApp<false>* app;
  extern std::thread wsThread;

  void start();
  void broadcast(nlohmann::json& j);
  void broadcast(const nlohmann::json& j);
  void requestState(std::string& topic);
  void requestCall(int devId, int reqId, std::string& proc, nlohmann::json& args, nlohmann::json& argsKw);
}

#endif /* WS_H */

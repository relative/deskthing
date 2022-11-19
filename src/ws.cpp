#include "ws.h"
#include "common.h"

namespace websocket {
  uWS::TemplatedApp<false>* app;
  std::thread wsThread;
}

void websocket::start() {
  using uws = uWS::WebSocket<false, true, PerSocketData>;
  app = new uWS::App();
  app->ws<PerSocketData>("/", {
    .compression = uWS::SHARED_COMPRESSOR,
    .maxPayloadLength = 16 * 1024 * 1024,
    .idleTimeout = 16,
    .maxBackpressure = 1 * 1024 * 1024,
    .closeOnBackpressureLimit = false,
    .resetIdleTimeoutOnSend = false,
    .sendPingsAutomatically = true,

    .upgrade = [&](auto* res, auto* req, auto* context) {
      // Ensure Origin header exists
      if (!req->getHeader("origin").data()) {
        res->close();
        return;
      }

      auto origin = std::string(req->getHeader("origin"));
      if (origin != "https://xpui.app.spotify.com" && origin != "https://open.spotify.com") {
        res->close();
        return;
      }

      res->template upgrade<PerSocketData>(
        { }, 
        req->getHeader("sec-websocket-key"), 
        req->getHeader("sec-websocket-protocol"), 
        req->getHeader("sec-websocket-extensions"), 
        context
      );
    },
    .open = [&](uws* ws) {
      ws->subscribe(WS_TOPIC);
    },
    .message = [&](uws* ws, std::string_view msg, uWS::OpCode opCode) {
      auto str = std::string(msg);
      try {
        auto j = json::parse(str);
        if (!j.contains("topic")) return;
        if (!j.contains("state")) return;
        auto topic = j["topic"].get<std::string>();
        auto state = j["state"];

        gCarThingManager->publish(topic, state);
      } catch (std::exception& ex) {
        fprintf(stderr, "[ERR] %s\n", ex.what());
      }
    },
    .close = [&](uws* ws, int code, std::string_view reason) {}
  }).listen("127.0.0.1", WS_PORT, [](auto* listen_s) {
    if (listen_s) {
      printf("WebSocket listening on port %d\n", WS_PORT);
    }
  });

  app->run();
}

void websocket::broadcast(json& j) {
  auto jsonStr = j.dump();
  app->publish(WS_TOPIC, jsonStr.c_str(), uWS::OpCode::TEXT, false);
}
void websocket::broadcast(const json& j) { broadcast(const_cast<json&>(j)); }
void websocket::requestState(std::string& topic) {
  broadcast({
    {"topic", topic},
  });
}

void websocket::requestCall(int reqId, std::string& proc, json& args, json& argsKw) {
  broadcast({
    {"id", reqId},
    {"proc", proc},
    {"args", args},
    {"argsKw", argsKw}
  });
}

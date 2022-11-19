#include "carthing.h"
#include "ia.h"
#include "../common.h"
#include "../ws.h"

std::shared_ptr<CarThing> CarThing::create(SOCKET s) {
  auto ct = std::shared_ptr<CarThing>(new CarThing(s));
  ct->thrd = std::thread(&CarThing::poll, ct);
  return ct;
}

CarThing::~CarThing() {
  dead = true;
  thrd.detach();
}

int CarThing::send(json& j) { 
  std::lock_guard guard(mutSend);
  return interapp::sendPacket(sock, j);
}
// Wrapper around the above function
int CarThing::send(const json& j) { return send(const_cast<json&>(j)); }

namespace {
  void dump(json& j) {
    auto msgCode = j[0].get<MessageCode>();
    // Logged out in their handlers
    if (msgCode == MessageCode::CALL || msgCode == MessageCode::SUBSCRIBE)
      return;

    printf("Received:\n%s\n", j.dump(4).c_str());
  }
}

void CarThing::subscribe(int reqId, std::string topic) {
  std::lock_guard guard(mutSub);
  int subId = ++subscriptionId;
  printf("[%i] Subscribing to %s (%i)\n", reqId, topic.c_str(), subId);
  subscriptions[topic] = subId;

  websocket::requestState(topic);

  send({
    MessageCode::SUBSCRIBED,
    reqId,
    subId
  });
}
void CarThing::unsubscribe(int reqId, int subId) {
  printf("[%i] Unsubscribing from %i\n", reqId, subId);
  std::lock_guard guard(mutSub);
  std::erase_if(subscriptions, [&](const auto& p) {
    auto const& [key, value] = p;
    return value == subId;
  });
  send({
    MessageCode::UNSUBSCRIBED,
    reqId
  });
}
void CarThing::processMessage(json& j) {
  if (!j.is_array()) return;
  if (j.size() == 0) throw std::runtime_error("Invalid WAMP message");
  
  auto msgCode = j[0].get<MessageCode>();

  dump(j);

  time_t now = time(0);
  std::stringstream ss;
  tm tm {};
  localtime_s(&tm, &now);
  ss << std::put_time(&tm, "%FT%T");
  
  if (!authed) {
    switch (msgCode) {
      case MessageCode::HELLO: {
        json chal = {
          {"authid", j[2]["authid"].get<std::string>()},
          {"authmethod", "wampcra"},
          {"authprovider", "spotify"},
          {"authrole", "app"},
          {"nonce", "dummy_nonce"},
          {"session", 0},
          {"timestamp", ss.str()}
        };
        send({
          MessageCode::CHALLENGE,
          "wampcra",
          {
            {"challenge", chal.dump()} // Yes
          }
        });
      } break;
      case MessageCode::AUTHENTICATE: {
        // You're supposed to verify the challenge
        authed = true;
        wampSessionId = 1; // And this is supposed to be random
        send({
          MessageCode::WELCOME,
          wampSessionId,
          {
            {"app_version", "8.7.82.94"},
            {"date_time", ss.str()},
            {"roles", {
              {"broker", json::object({})},
              {"dealer", json::object({})}
            }}
          }
        });
      } break;
      default: break;
    }
  } else {
    size_t jSz = j.size();

    unsigned int reqId = 0;
    auto type = j[1].type();
    if (type == json::value_t::number_unsigned) reqId = j[1].get<unsigned int>();

    switch(msgCode) {
      case MessageCode::SUBSCRIBE: {
        if (jSz != 4) throw std::runtime_error("Invalid SUBSCRIBE");
        if (j[2].type() != json::value_t::object) throw std::runtime_error("Options invalid type");
        if (j[3].type() != json::value_t::string) throw std::runtime_error("Topic invalid type");
        subscribe(reqId, j[3].get<std::string>());
      } break;
      case MessageCode::CALL: {
        if (jSz < 4 || jSz > 6) throw std::runtime_error("Invalid CALL");
        if (jSz >= 4 && j[3].type() != json::value_t::string) throw std::runtime_error("Procedure invalid type");
        if (jSz >= 5 && j[4].type() != json::value_t::array) throw std::runtime_error("Arguments invalid type");
        if (jSz >= 6 && j[5].type() != json::value_t::object) throw std::runtime_error("ArgumentsKw invalid type");
        std::string proc = j[3].get<std::string>();
        json args = json::array({});
        json argsKw = json::object({});
        if (jSz >= 5) args = j[4];
        if (jSz >= 6) argsKw = j[5];
        processRPC(reqId, proc, args, argsKw);
      } break;
      default: break;
    }
  }
}
void CarThing::poll() {
  std::vector<uint8_t> buf(MAX_BUF_LEN);
  std::vector<uint8_t> queue;
  int res = 0;

  while (!dead) {
    try {
      bool queueHasSize = queue.size() >= 4;

      bool canFulfillNow = false;
      if (!canFulfillNow && queueHasSize) {
        uint32_t requiredQueueSize = queue[3] | queue[2] << 8 | queue[1] << 16 | queue[0] << 24;
        canFulfillNow = queue.size() - 4 >= requiredQueueSize;
      }
      if (queue.empty() || !canFulfillNow) {
        res = platform::vrecv(sock, &buf);
        if (res > 0) {
          //printf("Received %d bytes\n", res);
          if (queue.empty() && buf.size() < 4)
            throw std::runtime_error("Missing size");
          
          std::move(buf.begin(), buf.begin() + res, std::back_inserter(queue));
        } else if (res == 0) {
          // No data pending
        } else {
          platform::vclose(sock);
          dead = true;
          throw std::runtime_error("Failed to poll CarThing");
        }
      }

      if (queueHasSize) {
        uint32_t wantedSize = queue[3] | queue[2] << 8 | queue[1] << 16 | queue[0] << 24;
        if (queue.size() - 4 >= wantedSize) {
          std::vector<uint8_t> vecMsg(wantedSize);
          std::move(queue.begin() + 4, queue.begin() + 4 + wantedSize, vecMsg.begin());

          auto x = json::from_msgpack(vecMsg);
          processMessage(x);

          queue.erase(queue.begin(), queue.begin() + 4 + wantedSize);
        }
      }
    } catch (std::exception& ex) {
      printf("[ERR] %s\n", ex.what());
    }
  }

  gCarThingManager->removeDeviceBySocket(sock);
}

int CarThing::publish(std::string& topic, json& details, int publicationId) {
  std::lock_guard guard(mutSub);
  int subId = subscriptions[topic];
  return send({
    MessageCode::EVENT,
    subId,
    publicationId,
    json::object({}),
    json::array({}),
    details
  });
}

static std::vector<std::string> forwardedProcedures {
  "com.spotify.superbird.volume.volume_up",
  "com.spotify.superbird.volume.volume_down",
  
  "com.spotify.superbird.pause",
  "com.spotify.set_playback_speed", //

  "com.spotify.superbird.graphql",
  "com.spotify.superbird.presets.set_preset",
  "com.spotify.superbird.seek_to",

  "com.spotify.get_saved",
  "com.spotify.get_children_of_item",

  "com.spotify.get_thumbnail_image",
  "com.spotify.get_image",
  "com.spotify.superbird.get_home",

  "com.spotify.superbird.remote_configuration"
};

int CarThing::processRPC(int reqId, std::string& proc, json& args, json& argsKw) {
  auto shouldForward = std::find(forwardedProcedures.begin(), forwardedProcedures.end(), proc) != forwardedProcedures.end();
  if (shouldForward) {
    websocket::requestCall(reqId, proc, args, argsKw);
    return 0;
  }
  
  if (proc != "com.spotify.superbird.instrumentation.log" && proc != "com.spotify.superbird.pitstop.log") {
    printf("[%i] RPC call received for %s(#args=%llu, #argsKw=%llu)\n", 
      reqId, proc.c_str(), args.size(), argsKw.size());
  }

  if (proc == "com.spotify.superbird.permissions") {
    return send({
      MessageCode::RESULT,
      reqId,
      json::object({}),
      {
        {"can_use_superbird", true}
      }
    });
  }

  return 0;
}

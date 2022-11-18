#include "carthing.h"
#include "ia.h"
#include "../common.h"

std::shared_ptr<CarThing> CarThing::create(SOCKET s) {
  auto ct = std::shared_ptr<CarThing>(new CarThing(s));
  ct->thrd = std::thread(&CarThing::poll, ct);
  return ct;
}

CarThing::~CarThing() {
  dead = true;
  thrd.detach();
}

int CarThing::send(json& j) { return interapp::sendPacket(sock, j); }
int CarThing::send(const json& j) { return interapp::sendPacket(sock, const_cast<json&>(j)); }

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
  int subId = ++subscriptionId;
  printf("[%i] Subscribing to %s (%i)\n", reqId, topic.c_str(), subId);
  subscriptions[topic] = subId;
  send({
    MessageCode::SUBSCRIBED,
    reqId,
    subId
  });
}
void CarThing::unsubscribe(int reqId, int subId) {
  printf("[%i] Unsubscribing from %i\n", reqId, subId);
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
              {"broker", {}},
              {"dealer", {}}
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
        json argsKw = {};
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

int CarThing::processRPC(int reqId, std::string proc, json& args, json& argsKw) {
  if (proc != "com.spotify.superbird.instrumentation.log" && proc != "com.spotify.superbird.pitstop.log") {
    printf("[%i] RPC call received for %s(#args=%llu, #argsKw=%llu)\n", 
      reqId, proc.c_str(), args.size(), argsKw.size());
  }

  if (proc == "com.spotify.superbird.voice.data") {
    printf("\n%s\n%s\n", args.dump(4).c_str(), argsKw.dump(4).c_str());
  }

  if (proc == "com.spotify.superbird.permissions") {
    return send({
      MessageCode::RESULT,
      reqId,
      {},
      {
        {"can_use_superbird", true}
      }
    });
  }

  return 0;
}

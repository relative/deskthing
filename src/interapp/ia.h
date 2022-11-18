#ifndef INTERAPP_IA_H
#define INTERAPP_IA_H

#include "../common.h"

// https://wamp-proto.org/wamp_bp_latest_ietf.html#name-message-codes-and-direction
// 4, 5, 49, 69 https://wamp-proto.org/wamp_latest_ietf.html#name-additional-messages
enum class MessageCode {
  HELLO = 1,
  WELCOME = 2,
  ABORT = 3,

  CHALLENGE = 4,
  AUTHENTICATE = 5,
  
  GOODBYE = 6,

  ERROR = 8,

  PUBLISH = 16,
  PUBLISHED = 17,

  SUBSCRIBE = 32,
  SUBSCRIBED = 33,
  UNSUBSCRIBE = 34,
  UNSUBSCRIBED = 35,
  EVENT = 36,

  CALL = 48,
  CANCEL = 49,
  RESULT = 50,

  REGISTER = 64,
  REGISTERED = 65,
  UNREGISTER = 66,
  UNREGISTERED = 67,
  INVOCATION = 68,
  INTERRUPT = 69,
  YIELD = 70
};

namespace interapp {
  int sendPacket(SOCKET s, json& j);
}

#endif /* INTERAPP_IA_H */

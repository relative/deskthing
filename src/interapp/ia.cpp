#include "../common.h"
#include "ia.h"

int interapp::sendPacket(SOCKET s, json& j) {
  std::vector<uint8_t> buf(4);
  auto mp = json::to_msgpack(j);
  uint32_t size = mp.size();
  buf[3] = size >> 0;
  buf[2] = size >> 8;
  buf[1] = size >> 16;
  buf[0] = size >> 24;
  std::move(mp.begin(), mp.end(), std::back_inserter(buf));
  return platform::vsend(s, buf);
}

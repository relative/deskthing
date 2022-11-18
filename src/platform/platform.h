#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdint>
#include <vector>

typedef unsigned long long SOCKET;

namespace platform {
  extern SOCKET s;
  bool startup();
  bool createSocket();
  bool registerSdpService();
  void startListening();
  bool shutdown();

  int vclose(SOCKET s);
  int vrecv(SOCKET s, std::vector<uint8_t>* vec, int flags = 0);
  int vsend(SOCKET s, std::vector<uint8_t>& vec, int flags = 0);
}

#endif /* PLATFORM_H */

#ifdef _WIN32
  #include <WinSock2.h>
  #include <Windows.h>
#endif
#include "common.h"
#include "ws.h"

int main(int argc, char** argv) {
  try {
    if (!platform::startup())
      throw std::runtime_error("Unexpected error while initializing");

    if (!platform::createSocket())
      throw std::runtime_error("Unexpected error while creating socket");

    if (!platform::registerSdpService())
      throw std::runtime_error("Unexpected error while registering SDP service");


    gCarThingManager = new CarThingManager();

    websocket::wsThread = std::thread(&websocket::start);

    platform::startListening();

    if (!platform::shutdown())
      throw std::runtime_error("Unexpected error while shutting down");
  } catch (wsa_error& e) {
    int wsaCode = WSAGetLastError();
    fprintf(stderr, "[ERR] %s : %d\n", e.what(), wsaCode);
  } catch (std::exception& e) {
    fprintf(stderr, "[ERR] %s\n", e.what());
  }
}

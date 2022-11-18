#include "../common.h"
#include "../interapp/ia.h"
#include <WinSock2.h>
#include <Windows.h>
#include <initguid.h>
#include <ws2bth.h>
#include <iostream>
#include <iterator>
#include <vector>

// Win32 

// Spotify UUID = E3CCCCCD-33B7-457D-A03C-AA1C54BF617F
DEFINE_GUID(GUID_SPOTIFY, 0xe3cccccd, 0x33b7, 0x457d, 0xa0, 0x3c, 0xaa, 0x1c, 0x54, 0xbf, 0x61, 0x7f);

namespace platform {
  SOCKET s = NULL;
}

bool platform::startup() {
  WSADATA wsaData {};
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    throw wsa_error("WSAStartup() failed");
  return true;
}

bool platform::createSocket() {
  s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
  if (s == INVALID_SOCKET)
    throw wsa_error("Failed to create socket");

  SOCKADDR_BTH sa {};
  int iSa = sizeof(SOCKADDR_BTH);
  sa.addressFamily = AF_BTH;
  sa.btAddr = 0;
  sa.serviceClassId = GUID_SPOTIFY;
  sa.port = BT_PORT_ANY;
  sockaddr* pAddr = (sockaddr*)&sa;

  if (bind(s, pAddr, sizeof(SOCKADDR_BTH)) != 0) {
    SAFE_CLOSE(s);
    throw wsa_error("Failed to bind socket");
  }

  if (listen(s, 2) != 0) {
    SAFE_CLOSE(s);
    throw wsa_error("Failed to listen socket");
  }

  if (getsockname(s, reinterpret_cast<SOCKADDR*>(&sa), &iSa) != 0) {
    SAFE_CLOSE(s);
    throw wsa_error("Failed to getsockname socket");
  }

  printf("Listening on RFCOMM channel %lu\n", sa.port);
  return true;
}

bool platform::registerSdpService() {
  WSAQUERYSETW qs;
  RtlSecureZeroMemory(&qs, sizeof(qs));
  
  ULONG version = BTH_SDP_VERSION;

  qs.dwSize = sizeof(WSAQUERYSETW);
  qs.lpszServiceInstanceName = const_cast<wchar_t*>(L"Spotify");
  qs.lpServiceClassId = const_cast<LPGUID>(&GUID_SPOTIFY);
  qs.dwNameSpace = NS_BTH;
  qs.dwNumberOfCsAddrs = 1;

  SOCKADDR_BTH sa = { 0 };
  int sa_len = sizeof(sa);
  if (getsockname(s, (SOCKADDR*)&sa, &sa_len) != 0)
    throw wsa_error("getsockname failed");
  CSADDR_INFO sockInfo = { 0 };
  sockInfo.iProtocol = BTHPROTO_RFCOMM;
  sockInfo.iSocketType = SOCK_STREAM;
  sockInfo.LocalAddr.lpSockaddr = (LPSOCKADDR)&sa;
  sockInfo.LocalAddr.iSockaddrLength = sizeof(sa);
  qs.lpcsaBuffer = &sockInfo;

  // Windows will automatically deregister our btsdp service once
  // our process is no longer running
  // We can't do it ourselves since we dont get a handle to our sdp registration
  if (WSASetServiceW(&qs, RNRSERVICE_REGISTER, 0) != 0)
    throw wsa_error("WSASetService failed");
  return true;
}

void platform::startListening() {
  SOCKET sock;
  SOCKADDR_BTH sa;
  int iSa = sizeof(sa);

  int res = 0;

  while (true) {
    iSa = sizeof(sa);
    sock = accept(s, (SOCKADDR*)&sa, &iSa);
    printf("Connection received from %04x%08lx to channel %lu\n", GET_NAP(sa.btAddr), GET_SAP(sa.btAddr), sa.port);
    gCarThingManager->addDevice(CarThing::create(sock));
  }
  return;
}

bool platform::shutdown() {
  if (s != NULL) {
    SAFE_CLOSE(s);
  }
  if (WSACleanup() != 0)
    throw wsa_error("WSACleanup() failed");
  return true;
}
int platform::vclose(SOCKET s) {
  return closesocket(s);
}
int platform::vrecv(SOCKET s, std::vector<uint8_t>* vec, int flags) {
  return recv(s, reinterpret_cast<char*>(vec->data()), vec->size(), 0);
}
int platform::vsend(SOCKET s, std::vector<uint8_t>& vec, int flags) {
  return send(s, reinterpret_cast<char*>(vec.data()), vec.size(), 0);
}
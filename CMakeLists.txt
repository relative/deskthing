cmake_minimum_required(VERSION 3.22)

# vcpkg (only Windows)
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "VCPKG_TARGET_TRIPLET" FORCE)
  message(STATUS "Target triplet for debug is " ${VCPKG_TARGET_TRIPLET})
else()
  set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "VCPKG_TARGET_TRIPLET" FORCE)
  message(STATUS "Target triplet for release is " ${VCPKG_TARGET_TRIPLET})
endif()


if(DEFINED ENV{VCPKG_ROOT})
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
else()
  message(STATUS "Vcpkg wasn't found, please follow instructions from `vcpkg integrate install` or define VCPKG_ROOT "
                 "in your environment")
endif()

project(deskthing)
set(VERBOSE 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_path(UWEBSOCKETS_INCLUDE_DIRS "uwebsockets/App.h")

find_library(LIBUV_LIB libuv)
find_library(USOCKETS_LIB usockets)

MESSAGE(STATUS "LIBUV_LIB:${LIBUV_LIB}")
MESSAGE(STATUS "USOCKETS_LIB:${USOCKETS_LIB}")

add_executable(deskthing 
  src/main.cpp src/common.h 
    
  src/interapp/carthing.cpp src/interapp/carthing.h
  src/interapp/ctmgr.cpp src/interapp/ctmgr.h
  src/interapp/ia.cpp src/interapp/ia.h

  src/platform/platform.h src/platform/win32.cpp

  src/ws.cpp src/ws.h
)

target_include_directories(deskthing PRIVATE ${UWEBSOCKETS_INCLUDE_DIRS})
target_link_libraries(deskthing PRIVATE 
  nlohmann_json::nlohmann_json
  ZLIB::ZLIB
  ${LIBUV_LIB} ${USOCKETS_LIB}
)
target_link_libraries(deskthing PRIVATE Shlwapi.lib wbemuuid.lib iphlpapi.lib Kernel32.lib Psapi.lib Userenv.lib WS2_32.lib)
set_property(TARGET deskthing PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
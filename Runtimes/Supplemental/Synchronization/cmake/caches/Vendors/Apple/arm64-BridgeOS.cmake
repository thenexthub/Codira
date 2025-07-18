if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  message(SEND_ERROR "CMAKE_OSX_DEPLOYMENT_TARGET not defined")
endif()

set(CMAKE_C_COMPILER_TARGET "arm64-apple-bridgeos${CMAKE_OSX_DEPLOYMENT_TARGET}" CACHE STRING "")
set(CMAKE_CXX_COMPILER_TARGET "arm64-apple-bridgeos${CMAKE_OSX_DEPLOYMENT_TARGET}" CACHE STRING "")
set(CMAKE_Codira_COMPILER_TARGET "arm64-apple-bridgeos${CMAKE_OSX_DEPLOYMENT_TARGET}" CACHE STRING "")

set(CodiraSynchronization_ARCH_SUBDIR arm64 CACHE STRING "")
set(CodiraSynchronization_PLATFORM_SUBDIR freestanding CACHE STRING "")
set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "")
set(CodiraSynchronization_SINGLE_THREADED_MODE YES CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/apple-common.cmake")

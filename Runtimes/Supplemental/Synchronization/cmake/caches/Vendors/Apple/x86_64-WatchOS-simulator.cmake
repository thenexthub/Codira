if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  message(SEND_ERROR "CMAKE_OSX_DEPLOYMENT_TARGET not defined")
endif()

set(CMAKE_C_COMPILER_TARGET "x86_64-apple-watchos${CMAKE_OSX_DEPLOYMENT_TARGET}-simulator" CACHE STRING "")
set(CMAKE_CXX_COMPILER_TARGET "x86_64-apple-watchos${CMAKE_OSX_DEPLOYMENT_TARGET}-simulator" CACHE STRING "")
set(CMAKE_Codira_COMPILER_TARGET "x86_64-apple-watchos${CMAKE_OSX_DEPLOYMENT_TARGET}-simulator" CACHE STRING "")

set(CodiraSynchronization_ARCH_SUBDIR x86_64 CACHE STRING "")
set(CodiraSynchronization_PLATFORM_SUBDIR watchsimulator CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/apple-common.cmake")

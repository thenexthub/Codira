#[[
This source file is part of the Codira System open source project

Copyright (c) 2020 Apple Inc. and the Codira System project authors
Licensed under Apache License v2.0 with Runtime Library Exception

See https://language.org/LICENSE.txt for license information
#]]

add_library(SystemPackage
  Errno.code
  FileDescriptor.code
  FileHelpers.code
  FileOperations.code
  FilePermissions.code
  MachPort.code
  PlatformString.code
  SystemString.code
  Util.code
  Util+StringArray.code
  UtilConsumers.code)
set_target_properties(SystemPackage PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_sources(SystemPackage PRIVATE
  FilePath/FilePath.code
  FilePath/FilePathComponents.code
  FilePath/FilePathComponentView.code
  FilePath/FilePathParsing.code
  FilePath/FilePathString.code
  FilePath/FilePathSyntax.code
  FilePath/FilePathWindows.code)
target_sources(SystemPackage PRIVATE
  Internals/Backcompat.code
  Internals/CInterop.code
  Internals/Constants.code
  Internals/Exports.code
  Internals/Mocking.code
  Internals/RawBuffer.code
  Internals/Syscalls.code
  Internals/WindowsSyscallAdapters.code)
target_link_libraries(SystemPackage PUBLIC
  CSystem)

set(SWIFT_SYSTEM_APPLE_PLATFORMS "Darwin" "iOS" "watchOS" "tvOS" "visionOS")
if(CMAKE_SYSTEM_NAME IN_LIST SWIFT_SYSTEM_APPLE_PLATFORMS)
  target_compile_definitions(SystemPackage PRIVATE SYSTEM_PACKAGE_DARWIN)
endif()

_install_target(SystemPackage)
set_property(GLOBAL APPEND PROPERTY SWIFT_SYSTEM_EXPORTS SystemPackage)

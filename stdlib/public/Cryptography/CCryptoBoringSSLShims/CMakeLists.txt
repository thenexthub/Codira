##===----------------------------------------------------------------------===##
##
## This source file is part of the CodiraCrypto open source project
##
## Copyright (c) 2021 Apple Inc. and the CodiraCrypto project authors
## Licensed under Apache License v2.0
##
## See LICENSE.txt for license information
## See CONTRIBUTORS.txt for the list of CodiraCrypto project authors
##
## SPDX-License-Identifier: Apache-2.0
##
##===----------------------------------------------------------------------===##

add_library(CCryptoBoringSSLShims STATIC
  "shims.c"
)

target_include_directories(CCryptoBoringSSLShims PUBLIC
  include
  $<TARGET_PROPERTY:CCryptoBoringSSL,INCLUDE_DIRECTORIES>)

target_link_libraries(CCryptoBoringSSLShims PUBLIC
  CCryptoBoringSSL)

set_target_properties(CCryptoBoringSSLShims PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})

set_property(GLOBAL APPEND PROPERTY SWIFT_CRYPTO_EXPORTS CCryptoBoringSSLShims)

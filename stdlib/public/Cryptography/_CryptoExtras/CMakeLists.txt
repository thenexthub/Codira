##===----------------------------------------------------------------------===##
##
## This source file is part of the CodiraCrypto open source project
##
## Copyright (c) 2023 Apple Inc. and the CodiraCrypto project authors
## Licensed under Apache License v2.0
##
## See LICENSE.txt for license information
## See CONTRIBUTORS.txt for the list of CodiraCrypto project authors
##
## SPDX-License-Identifier: Apache-2.0
##
##===----------------------------------------------------------------------===##

add_library(_CryptoExtras
  "AES/AES_CBC.code"
  "AES/AES_CFB.code"
  "AES/AES_CTR.code"
  "AES/AES_GCM_SIV.code"
  "AES/Block Function.code"
  "AES/BoringSSL/AES_CFB_boring.code"
  "AES/BoringSSL/AES_CTR_boring.code"
  "AES/BoringSSL/AES_GCM_SIV_boring.code"
  "ARC/ARC+API.code"
  "ARC/ARC.code"
  "ARC/ARCCredential.code"
  "ARC/ARCEncoding.code"
  "ARC/ARCPrecredential.code"
  "ARC/ARCPresentation.code"
  "ARC/ARCRequest.code"
  "ARC/ARCResponse.code"
  "ARC/ARCServer.code"
  "ChaCha20CTR/BoringSSL/ChaCha20CTR_boring.code"
  "ChaCha20CTR/ChaCha20CTR.code"
  "EC/ObjectIdentifier.code"
  "EC/PKCS8DERRepresentation.code"
  "EC/PKCS8PrivateKey.code"
  "EC/RFC8410AlgorithmIdentifier.code"
  "ECToolbox/BoringSSL/ECToolbox_boring.code"
  "ECToolbox/ECToolbox.code"
  "H2G/HashToField.code"
  "Key Derivation/KDF.code"
  "Key Derivation/PBKDF2/BoringSSL/PBKDF2_boring.code"
  "Key Derivation/PBKDF2/BoringSSL/PBKDF2_commoncrypto.code"
  "Key Derivation/PBKDF2/PBKDF2.code"
  "Key Derivation/Scrypt/BoringSSL/Scrypt_boring.code"
  "Key Derivation/Scrypt/Scrypt.code"
  "OPRFs/OPRF.code"
  "OPRFs/OPRFClient.code"
  "OPRFs/OPRFServer.code"
  "OPRFs/VOPRF+API.code"
  "OPRFs/VOPRFClient.code"
  "OPRFs/VOPRFServer.code"
  "RSA/RSA+BlindSigning.code"
  "RSA/RSA.code"
  "RSA/RSA_boring.code"
  "Util/BoringSSLHelpers.code"
  "Util/CryptoKitErrors_boring.code"
  "Util/Data+Extensions.code"
  "Util/DigestType.code"
  "Util/Error.code"
  "Util/I2OSP.code"
  "Util/IntegerEncoding.code"
  "Util/PEMDocument.code"
  "Util/PrettyBytes.code"
  "Util/SubjectPublicKeyInfo.code"
  "ZKPs/DLEQ.code"
  "ZKPs/Prover.code"
  "ZKPs/Verifier.code"
  "ZKPs/ZKPToolbox.code"
)

target_compile_options(_CryptoExtras PRIVATE -DCRYPTO_IN_SWIFTPM)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_compile_options(_CryptoExtras PRIVATE -DCRYPTO_IN_SWIFTPM_FORCE_BUILD_API)
endif()

target_include_directories(_CryptoExtras PRIVATE
  $<TARGET_PROPERTY:CCryptoBoringSSL,INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:CCryptoBoringSSLShims,INCLUDE_DIRECTORIES>)

target_link_libraries(_CryptoExtras PUBLIC
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>
  Crypto
  CryptoBoringWrapper
  CCryptoBoringSSL
  CCryptoBoringSSLShims)

target_compile_options(_CryptoExtras PRIVATE ${SWIFT_CRYPTO_COMPILE_OPTIONS})

target_link_options(_CryptoExtras PRIVATE
    "$<$<PLATFORM_ID:Darwin>:SHELL:-Xlinker -framework -Xlinker Security>")

set_target_properties(_CryptoExtras PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})

set_property(GLOBAL APPEND PROPERTY SWIFT_CRYPTO_EXPORTS _CryptoExtras)

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

add_library(Crypto
  "AEADs/AES/GCM/AES-GCM.code"
  "AEADs/AES/GCM/BoringSSL/AES-GCM_boring.code"
  "AEADs/ChachaPoly/BoringSSL/ChaChaPoly_boring.code"
  "AEADs/ChachaPoly/ChaChaPoly.code"
  "AEADs/Cipher.code"
  "AEADs/Nonces.code"
  "ASN1/ASN1.code"
  "ASN1/Basic ASN1 Types/ASN1Any.code"
  "ASN1/Basic ASN1 Types/ASN1BitString.code"
  "ASN1/Basic ASN1 Types/ASN1Boolean.code"
  "ASN1/Basic ASN1 Types/ASN1Identifier.code"
  "ASN1/Basic ASN1 Types/ASN1Integer.code"
  "ASN1/Basic ASN1 Types/ASN1Null.code"
  "ASN1/Basic ASN1 Types/ASN1OctetString.code"
  "ASN1/Basic ASN1 Types/ASN1Strings.code"
  "ASN1/Basic ASN1 Types/ArraySliceBigint.code"
  "ASN1/Basic ASN1 Types/GeneralizedTime.code"
  "ASN1/Basic ASN1 Types/ObjectIdentifier.code"
  "ASN1/ECDSASignature.code"
  "ASN1/PEMDocument.code"
  "ASN1/PKCS8PrivateKey.code"
  "ASN1/SEC1PrivateKey.code"
  "ASN1/SubjectPublicKeyInfo.code"
  "CryptoError_boring.code"
  "CryptoKitErrors.code"
  "Digests/BoringSSL/Digest_boring.code"
  "Digests/Digest.code"
  "Digests/Digests.code"
  "Digests/HashFunctions.code"
  "Digests/HashFunctions_SHA2.code"
  "HPKE/Ciphersuite/HPKE-AEAD.code"
  "HPKE/Ciphersuite/HPKE-Ciphersuite.code"
  "HPKE/Ciphersuite/HPKE-KDF.code"
  "HPKE/Ciphersuite/HPKE-KexKeyDerivation.code"
  "HPKE/Ciphersuite/HPKE-LabeledExtract.code"
  "HPKE/Ciphersuite/HPKE-Utils.code"
  "HPKE/Ciphersuite/KEM/Conformances/DHKEM.code"
  "HPKE/Ciphersuite/KEM/Conformances/HPKE-KEM-Curve25519.code"
  "HPKE/Ciphersuite/KEM/Conformances/HPKE-NIST-EC-KEMs.code"
  "HPKE/Ciphersuite/KEM/HPKE-KEM.code"
  "HPKE/HPKE-Errors.code"
  "HPKE/HPKE.code"
  "HPKE/Key Schedule/HPKE-Context.code"
  "HPKE/Key Schedule/HPKE-KeySchedule.code"
  "HPKE/Modes/HPKE-Modes.code"
  "Insecure/Insecure.code"
  "Insecure/Insecure_HashFunctions.code"
  "KEM/KEM.code"
  "Key Agreement/BoringSSL/ECDH_boring.code"
  "Key Agreement/DH.code"
  "Key Agreement/ECDH.code"
  "Key Derivation/HKDF.code"
  "Key Wrapping/AESWrap.code"
  "Key Wrapping/BoringSSL/AESWrap_boring.code"
  "Keys/EC/BoringSSL/Ed25519_boring.code"
  "Keys/EC/BoringSSL/NISTCurvesKeys_boring.code"
  "Keys/EC/BoringSSL/X25519Keys_boring.code"
  "Keys/EC/Curve25519.code"
  "Keys/EC/Ed25519Keys.code"
  "Keys/EC/NISTCurvesKeys.code"
  "Keys/EC/X25519Keys.code"
  "Keys/Symmetric/SymmetricKeys.code"
  "Message Authentication Codes/HMAC/HMAC.code"
  "Message Authentication Codes/MACFunctions.code"
  "Message Authentication Codes/MessageAuthenticationCode.code"
  "PRF/AES.code"
  "Signatures/BoringSSL/ECDSASignature_boring.code"
  "Signatures/BoringSSL/ECDSA_boring.code"
  "Signatures/BoringSSL/EdDSA_boring.code"
  "Signatures/ECDSA.code"
  "Signatures/Ed25519.code"
  "Signatures/Signature.code"
  "Util/BoringSSL/CryptoKitErrors_boring.code"
  "Util/BoringSSL/RNG_boring.code"
  "Util/BoringSSL/SafeCompare_boring.code"
  "Util/BoringSSL/Zeroization_boring.code"
  "Util/PrettyBytes.code"
  "Util/SafeCompare.code"
  "Util/SecureBytes.code"
  "Util/Zeroization.code"
)

target_compile_definitions(Crypto PRIVATE
  "$<$<COMPILE_LANGUAGE:Codira>:CRYPTO_IN_SWIFTPM>")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_compile_definitions(Crypto PRIVATE
    "$<$<COMPILE_LANGUAGE:Codira>:CRYPTO_IN_SWIFTPM_FORCE_BUILD_API>")
endif()

target_include_directories(Crypto PRIVATE
  $<TARGET_PROPERTY:CCryptoBoringSSL,INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:CCryptoBoringSSLShims,INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:CryptoBoringWrapper,INCLUDE_DIRECTORIES>)
target_link_libraries(Crypto PUBLIC
  $<$<NOT:$<PLATFORM_ID:Darwin>>:dispatch>
  $<$<NOT:$<PLATFORM_ID:Darwin>>:Foundation>
  CCryptoBoringSSL
  CCryptoBoringSSLShims
  CryptoBoringWrapper)
target_compile_options(Crypto PRIVATE ${SWIFT_CRYPTO_COMPILE_OPTIONS})
set_target_properties(Crypto PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})

_install_target(Crypto)
set_property(GLOBAL APPEND PROPERTY SWIFT_CRYPTO_EXPORTS Crypto)

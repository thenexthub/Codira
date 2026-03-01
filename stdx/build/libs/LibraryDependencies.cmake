# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

# Defining dependencies for all backends, BACKEND_TYPE is defined outside where this file is included

set(LOGGER_DEPENDENCIES
    cangjie${BACKEND_TYPE}Log
    cangjie${BACKEND_TYPE}JsonStream)

set(ENCODING_JSON_DEPENDENCIES
    cangjie${BACKEND_TYPE}Serialization)

set(CRYPTO_COMMON_DEPENDENCIES
    cangjie${BACKEND_TYPE}Base64)

set(CRYPTO_KIT_DEPENDENCIES
    cangjie${BACKEND_TYPE}CryptoCommon
    cangjie${BACKEND_TYPE}Crypto
    cangjie${BACKEND_TYPE}X509
    cangjie${BACKEND_TYPE}Keys)

set(CRYPTO_DEPENDENCIES
    cangjie${BACKEND_TYPE}Digest
    cangjie${BACKEND_TYPE}CryptoCommon)

set(CRYPTO_DIGEST_DEPENDENCIES
    cangjie${BACKEND_TYPE}Hex
    cangjie${BACKEND_TYPE}CryptoCommon)

set(CRYPTO_KEYS_DEPENDENCIES
    cangjie${BACKEND_TYPE}Digest
    cangjie${BACKEND_TYPE}Hex
    cangjie${BACKEND_TYPE}CryptoCommon)

set(X509_DEPENDENCIES
    cangjie${BACKEND_TYPE}Hex
    cangjie${BACKEND_TYPE}Base64
    cangjie${BACKEND_TYPE}Crypto
    cangjie${BACKEND_TYPE}Keys
    cangjie${BACKEND_TYPE}CryptoCommon)

set(NET_TLS_DEPENDENCIES
    cangjie${BACKEND_TYPE}Hex
    cangjie${BACKEND_TYPE}Base64
    cangjie${BACKEND_TYPE}X509
    cangjie${BACKEND_TYPE}Digest
    cangjie${BACKEND_TYPE}CryptoCommon
    cangjie${BACKEND_TYPE}TlsCommon)

set(NET_TLS_COMMON_DEPENDENCIES
    cangjie${BACKEND_TYPE}CryptoCommon)

set(NET_HTTP_DEPENDENCIES
    cangjie${BACKEND_TYPE}Base64
    cangjie${BACKEND_TYPE}Url
    cangjie${BACKEND_TYPE}Log
    cangjie${BACKEND_TYPE}Logger
    cangjie${BACKEND_TYPE}TlsCommon
    cangjie${BACKEND_TYPE}CryptoCommon)

set(UNITTEST_DATA_DEPENDENCIES
    cangjie${BACKEND_TYPE}Serialization
    cangjie${BACKEND_TYPE}Json)
     
set(STRING_INTERN_DEPENDENCIES)

set(ASPECTCODE_DEPENDENCIES)

if(NOT CODIRA_BUILD_WITHOUT_EFFECT_HANDLERS)
set(EFFECT_DEPENDENCIES)
endif()

set(ACTORS_DEPENDENCIES)

set(ACTORS_MACROS_DEPENDENCIES)

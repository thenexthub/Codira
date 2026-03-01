# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10)

project(openssl CXX C ASM)

set(TARGET_OUT_DIR ${CMAKE_BINARY_DIR}/third_party/openssl)

set(OPENSSL_INTERNAL_CFLAGS_CC
    -Wall

    # ../../third_party/openssl/crypto/o_str.c:309:9: error: incompatible integer to pointer conversion assigning to 'char *' from 'int' [-Werror,-Wint-conversion]
    #     err = strerror_r(errnum, buf, buflen);
    #         ^ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    -Wno-error=int-conversion

    # ../../third_party/openssl/crypto/bn/bn_exp.c:382:38: error: shift count >= width of type [-Werror,-Wshift-count-overflow]
    #     if (m->d[j - 1] & (((BN_ULONG)1) << (BN_BITS2 - 1))) {
    #                                      ^  ~~~~~~~~~~~~~~
    -Wno-error=shift-count-overflow

    -Wno-error=implicit-fallthrough
    -Wno-error=sign-compare

    # Fix llvm-15 build error
    -Wno-unused-but-set-variable
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND OPENSSL_INTERNAL_CFLAGS_CC
        # ../../third_party/openssl/crypto/bn/bn_conv.c:92:34: error: implicit conversion from 'unsigned long long' to 'unsigned long' changes value from 10000000000000000000 to 2313682944 [-Werror,-Wconstant-conversion]
        #             *lp = BN_div_word(t, BN_DEC_CONV);
        #                   ~~~~~~~~~~~    ^~~~~~~~~~~
        -Wno-error=constant-conversion

        # ../../third_party/openssl/crypto/conf/conf_def.c:31:11: error: 'stat' macro redefined [-Werror,-Wmacro-redefined]
        # #  define stat    _stat
        #           ^
        # ../../prebuilts/mingw-w64/ohos/linux-x86_64/clang-mingw/x86_64-w64-mingw32/include/sys/stat.h:279:9: note: previous definition is here
        # #define stat _stat64
        #         ^
        # 1 error generated.
        -Wno-error=macro-redefined
    )
endif()

set(CRYPTO_CONFIG_COMMON_CFLAGS_CC
    -Wa,--noexecstack
)

list(APPEND CRYPTO_CONFIG_COMMON_CFLAGS_CC
    ${OPENSSL_INTERNAL_CFLAGS_CC}
)

set(CRYPTO_CONFIG_COMMON_DEFINITIONS
    "-DNDEBUG"
    "-DOPENSSL_BUILDING_OPENSSL"
    "-DOPENSSL_CPUID_OBJ"
    "-DOPENSSL_PIC"
    "-DENGINESDIR=\"\""
    "-DMODULESDIR=\"\""
    "-DOPENSSLDIR=\"\""
    "-DSTATIC_LEGACY"
)

# platform-specific compiler options and definitions

set(CRYPTO_CONFIG_LINUX_ARMV4_CFLAGS_CC
    "-fPIC"
    "-pthread"
)
set(CRYPTO_CONFIG_LINUX_ARMV4_DEFINITIONS
    "-DOPENSSL_USE_NODELETE"
    "-DAES_ASM"
    "-DBSAES_ASM"
    "-DECP_NISTZ256_ASM"
    "-DGHASH_ASM"
    "-DKECCAK1600_ASM"
    "-DOPENSSL_BN_ASM_GF2m"
    "-DOPENSSL_BN_ASM_MONT"
    "-DPOLY1305_ASM"
    "-DSHA1_ASM"
    "-DSHA256_ASM"
    "-DSHA512_ASM"
)

set(CRYPTO_CONFIG_LINUX_AARCH64_CFLAGS_CC
    "-fPIC"
    "-pthread"
)
set(CRYPTO_CONFIG_LINUX_AARCH64_DEFINITIONS
    "-DOPENSSL_USE_NODELETE"
    "-DECP_NISTZ256_ASM"
    "-DKECCAK1600_ASM"
    "-DOPENSSL_BN_ASM_MONT"
    "-DPOLY1305_ASM"
    "-DSHA1_ASM"
    "-DSHA256_ASM"
    "-DSHA512_ASM"
    "-DVPAES_ASM"
)

set(CRYPTO_CONFIG_DARWIN64_X86_64_CC_CFLAGS_CC
    "-fPIC"
)
set(CRYPTO_CONFIG_DARWIN64_X86_64_CC_DEFINITIONS
    "-DL_ENDIAN"
    "-D_REENTRANT"
    "-DOPENSSL_IA32_SSE2"
    "-DAES_ASM"
    "-DBSAES_ASM"
    "-DCMLL_ASM"
    "-DECP_NISTZ256_ASM"
    "-DGHASH_ASM"
    "-DKECCAK1600_ASM"
    "-DMD5_ASM"
    "-DOPENSSL_BN_ASM_GF2m"
    "-DOPENSSL_BN_ASM_MONT"
    "-DOPENSSL_BN_ASM_MONT5"
    "-DPADLOCK_ASM"
    "-DPOLY1305_ASM"
    "-DSHA1_ASM"
    "-DSHA256_ASM"
    "-DSHA512_ASM"
    "-DVPAES_ASM"
    "-DWHIRLPOOL_ASM"
    "-DX25519_ASM"
)

set(CRYPTO_CONFIG_DARWIN64_ARM64_CC_CFLAGS_CC
    "-fPIC"
)
set(CRYPTO_CONFIG_DARWIN64_ARM64_CC_DEFINITIONS
    "-DL_ENDIAN"
    "-D_REENTRANT"
    "-DECP_NISTZ256_ASM"
    "-DKECCAK1600_ASM"
    "-DOPENSSL_BN_ASM_MONT"
    "-DPOLY1305_ASM"
    "-DSHA1_ASM"
    "-DSHA256_ASM"
    "-DSHA512_ASM"
    "-DVPAES_ASM"
)

set(CRYPTO_CONFIG_LINUX_X86_64_CFLAGS_CC
    "-fPIC"
    "-m64"
    "-pthread"
)
set(CRYPTO_CONFIG_LINUX_X86_64_DEFINITIONS
    "-DL_ENDIAN"
    "-DOPENSSL_IA32_SSE2"
    "-DOPENSSL_USE_NODELETE"
    "-DAES_ASM"
    "-DBSAES_ASM"
    "-DCMLL_ASM"
    "-DECP_NISTZ256_ASM"
    "-DGHASH_ASM"
    "-DKECCAK1600_ASM"
    "-DMD5_ASM"
    "-DOPENSSL_BN_ASM_GF2m"
    "-DOPENSSL_BN_ASM_MONT"
    "-DOPENSSL_BN_ASM_MONT5"
    "-DPADLOCK_ASM"
    "-DPOLY1305_ASM"
    "-DSHA1_ASM"
    "-DSHA256_ASM"
    "-DSHA512_ASM"
    "-DVPAES_ASM"
    "-DWHIRLPOOL_ASM"
    "-DX25519_ASM"
)

set(CRYPTO_CONFIG_LINUX_X86_CFLAGS_CC
    ""
)
set(CRYPTO_CONFIG_LINUX_X86_DEFINITIONS
    ""
)

set(CRYPTO_CONFIG_MINGW64_CFLAGS_CC
  "-m64"
)
set(CRYPTO_CONFIG_MINGW64_DEFINITIONS
  "-D_MT"
  "-D_UNICODE"
  "-DL_ENDIAN"
  "-DOPENSSL_IA32_SSE2"
  "-DUNICODE"
  "-DWIN32_LEAN_AND_MEAN"
  "-DAES_ASM"
  "-DBSAES_ASM"
  "-DCMLL_ASM"
  "-DECP_NISTZ256_ASM"
  "-DGHASH_ASM"
  "-DKECCAK1600_ASM"
  "-DMD5_ASM"
  "-DOPENSSL_BN_ASM_GF2m"
  "-DOPENSSL_BN_ASM_MONT"
  "-DOPENSSL_BN_ASM_MONT5"
  "-DPADLOCK_ASM"
  "-DPOLY1305_ASM"
  "-DSHA1_ASM"
  "-DSHA256_ASM"
  "-DSHA512_ASM"
  "-DVPAES_ASM"
  "-DWHIRLPOOL_ASM"
  "-DX25519_ASM"
)

set(CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC "")
set(CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS "")
set(OPENSSL_SELECTED_PLATFORM "")

if (PANDA_TARGET_WINDOWS)
    set(OPENSSL_SELECTED_PLATFORM "mingw64")
    list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC
        ${CRYPTO_CONFIG_MINGW64_CFLAGS_CC}
    )
    list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS
        ${CRYPTO_CONFIG_MINGW64_DEFINITIONS}
    )
elseif (PANDA_TARGET_MACOS)
    if (PANDA_TARGET_AMD64)
        set(OPENSSL_SELECTED_PLATFORM "darwin64-x86_64-cc")
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC
            ${CRYPTO_CONFIG_DARWIN64_X86_64_CC_CFLAGS_CC}
        )
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS
            ${CRYPTO_CONFIG_DARWIN64_X86_64_CC_DEFINITIONS}
        )
    elseif (PANDA_TARGET_ARM64)
        set(OPENSSL_SELECTED_PLATFORM "darwin64-arm64-cc")
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC
            ${CRYPTO_CONFIG_DARWIN64_ARM64_CC_CFLAGS_CC}
        )
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS
            ${CRYPTO_CONFIG_DARWIN64_ARM64_CC_DEFINITIONS}
        )
    endif()
else()
    if (PANDA_TARGET_AMD64)
        set(OPENSSL_SELECTED_PLATFORM "linux-x86_64")
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC
            ${CRYPTO_CONFIG_LINUX_X86_64_CFLAGS_CC}
        )
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS
            ${CRYPTO_CONFIG_LINUX_X86_64_DEFINITIONS}
        )
    elseif (PANDA_TARGET_X86)
        set(OPENSSL_SELECTED_PLATFORM "linux-x86")
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC
            ${CRYPTO_CONFIG_LINUX_X86_CFLAGS_CC}
        )
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS
            ${CRYPTO_CONFIG_LINUX_X86_DEFINITIONS}
        )
    elseif (PANDA_TARGET_ARM32)
        set(OPENSSL_SELECTED_PLATFORM "linux-armv4")
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC
            ${CRYPTO_CONFIG_LINUX_ARMV4_CFLAGS_CC}
        )
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS
            ${CRYPTO_CONFIG_LINUX_ARMV4_DEFINITIONS}
        )
    elseif (PANDA_TARGET_ARM64)
        set(OPENSSL_SELECTED_PLATFORM "linux-aarch64")
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC
            ${CRYPTO_CONFIG_LINUX_AARCH64_CFLAGS_CC}
        )
        list(APPEND CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS
            ${CRYPTO_CONFIG_LINUX_AARCH64_DEFINITIONS}
        )
    endif()
endif()

set(TARGET_GENERATED_DIR ${TARGET_OUT_DIR}/generated)
if (NOT IS_DIRECTORY ${TARGET_GENERATED_DIR} AND NOT CROSS_VALUES_CONFIG)
    # Openssl generate some assembly codes before it is building.
    # The sources list in include the assembly codes.
    execute_process(COMMAND python3
                        ${OPENSSL_ROOT}/run_command.py
                        ${OPENSSL_ROOT}/make_openssl_build_all_generated.sh
                        ${OPENSSL_ROOT}
                        ${TARGET_GENERATED_DIR}
                        ${OPENSSL_SELECTED_PLATFORM}
                    WORKING_DIRECTORY ${PANDA_ROOT}
                    RESULT_VARIABLE OPENSSL_GEN_OK)
    if (NOT OPENSSL_GEN_OK EQUAL 0)
        message(FATAL_ERROR "Unable to generate openssl files")
    endif()
endif()

set(OPENSSL_CFLAGS_CC "")
list(APPEND OPENSSL_CFLAGS_CC
    ${CRYPTO_CONFIG_COMMON_CFLAGS_CC}
    ${CRYPTO_CONFIG_CURRENT_PLATFORM_CFLAGS_CC}
)

set(OPENSSL_DEFINITIONS "")
list(APPEND OPENSSL_DEFINITIONS
    ${CRYPTO_CONFIG_COMMON_DEFINITIONS}
    ${CRYPTO_CONFIG_CURRENT_PLATFORM_DEFINITIONS}
)

set(OPENSSL_SELECTED_PLATFORM_FULL_PATH
    ${TARGET_GENERATED_DIR}/${OPENSSL_SELECTED_PLATFORM}
)

set(CRYPTO_CONFIG_COMMON_PRIVATE_INCLUDES
    ${OPENSSL_ROOT}
    ${OPENSSL_ROOT}/apps/include
    ${OPENSSL_ROOT}/crypto
    ${OPENSSL_ROOT}/crypto/bn
    ${OPENSSL_ROOT}/crypto/ec
    ${OPENSSL_ROOT}/crypto/ec/curve448
    ${OPENSSL_ROOT}/crypto/modes
    ${OPENSSL_ROOT}/crypto/rsa
    ${OPENSSL_ROOT}/include
    ${OPENSSL_ROOT}/providers/common/include
    ${OPENSSL_ROOT}/providers/common/include/prov
    ${OPENSSL_ROOT}/providers/implementations/include
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/apps
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/include
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/include/crypto
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/include/openssl
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/include
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/include/prov
)
set(CRYPTO_CONFIG_COMMON_PUBLIC_INCLUDES
    ${OPENSSL_ROOT}/include
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/include
)

set(LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_ARMV4_SOURCES
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aes-armv4.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesv8-armx.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/bsaes-armv7.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/armv4cpuid.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/armv4-gf2m.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/armv4-mont.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/chacha/chacha-armv4.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/ecp_nistz256-armv4.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/ghash-armv4.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/ghashv8-armx.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/poly1305/poly1305-armv4.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/keccak1600-armv4.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-armv4-large.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-armv4.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha512-armv4.S
)
set(LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_AARCH64_SOURCES
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesv8-armx.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/vpaes-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/arm64cpuid.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/armv8-mont.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/chacha/chacha-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/ecp_nistz256-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/aes-gcm-armv8_64.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/ghashv8-armx.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/poly1305/poly1305-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/keccak1600-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha512-armv8.S
)
set(LIBCRYPTO_BUILD_ALL_GENERATED_DARWIN64_X86_64_CC_SOURCES
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-sha1-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-sha256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/bsaes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/vpaes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-avx2.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-avx512.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-gf2m.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-mont.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-mont5.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/camellia/cmll-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/chacha/chacha-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/ecp_nistz256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/x25519-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/md5/md5-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/aesni-gcm-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/ghash-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/poly1305/poly1305-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/rc4/rc4-md5-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/rc4/rc4-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/keccak1600-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha512-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/whrlpool/wp-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/x86_64cpuid.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/engines/e_padlock-x86_64.s
)
set(LIBCRYPTO_BUILD_ALL_GENERATED_DARWIN64_ARM64_CC_SOURCES
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesv8-armx.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/vpaes-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/arm64cpuid.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/armv8-mont.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/chacha/chacha-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/ecp_nistz256-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/aes-gcm-armv8_64.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/ghashv8-armx.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/poly1305/poly1305-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/keccak1600-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-armv8.S
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha512-armv8.S
)
set(LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_X86_64_SOURCES
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-sha1-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-sha256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/bsaes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/vpaes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-avx2.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-avx512.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-gf2m.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-mont.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-mont5.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/camellia/cmll-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/chacha/chacha-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/ecp_nistz256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/x25519-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/md5/md5-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/aesni-gcm-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/ghash-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/poly1305/poly1305-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/rc4/rc4-md5-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/rc4/rc4-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/keccak1600-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha512-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/whrlpool/wp-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/x86_64cpuid.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/engines/e_padlock-x86_64.s
)
set(LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_X86_SOURCES
    ""
)
set(LIBCRYPTO_BUILD_ALL_GENERATED_MINGW64_SOURCES
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-sha1-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-sha256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/aesni-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/bsaes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/aes/vpaes-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-avx2.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-avx512.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/rsaz-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-gf2m.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-mont.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/bn/x86_64-mont5.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/camellia/cmll-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/chacha/chacha-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/ecp_nistz256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/ec/x25519-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/md5/md5-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/aesni-gcm-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/modes/ghash-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/poly1305/poly1305-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/rc4/rc4-md5-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/rc4/rc4-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/keccak1600-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha1-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-mb-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha256-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/sha/sha512-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/whrlpool/wp-x86_64.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/crypto/x86_64cpuid.s
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/engines/e_padlock-x86_64.s
)

if (NOT CROSS_VALUES_CONFIG)
    set(LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
        ""
    )
    if ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-armv4")
        list(APPEND LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
            ${LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_ARMV4_SOURCES}
        )
    elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-aarch64")
        list(APPEND LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
            ${LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_AARCH64_SOURCES}
        )
    elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "darwin64-x86_64-cc")
        list(APPEND LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
            ${LIBCRYPTO_BUILD_ALL_GENERATED_DARWIN64_X86_64_CC_SOURCES}
        )
    elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "darwin64-arm64-cc")
        list(APPEND LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
            ${LIBCRYPTO_BUILD_ALL_GENERATED_DARWIN64_ARM64_CC_SOURCES}
        )
    elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-x86_64")
        list(APPEND LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
            ${LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_X86_64_SOURCES}
        )
    elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-x86")
        list(APPEND LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
            ${LIBCRYPTO_BUILD_ALL_GENERATED_LINUX_X86_SOURCES}
        )
    elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "mingw64")
        list(APPEND LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
            ${LIBCRYPTO_BUILD_ALL_GENERATED_MINGW64_SOURCES}
        )
    endif()

    set(LIBCOMMON_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
        ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/der/der_digests_gen.c
        ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/der/der_dsa_gen.c
        ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/der/der_ec_gen.c
        ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/der/der_ecx_gen.c
        ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/der/der_rsa_gen.c
        ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/der/der_wrap_gen.c
    )

    set(LIBDEFAULT_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES
    ${OPENSSL_SELECTED_PLATFORM_FULL_PATH}/providers/common/der/der_sm2_gen.c
    )
endif()

set(LIBCOMMON_COMMON_SOURCES
    ${OPENSSL_ROOT}/providers/common/der/der_dsa_key.c
    ${OPENSSL_ROOT}/providers/common/der/der_dsa_sig.c
    ${OPENSSL_ROOT}/providers/common/der/der_ec_key.c
    ${OPENSSL_ROOT}/providers/common/der/der_ec_sig.c
    ${OPENSSL_ROOT}/providers/common/der/der_ecx_key.c
    ${OPENSSL_ROOT}/providers/common/der/der_rsa_key.c
    ${OPENSSL_ROOT}/providers/common/provider_ctx.c
    ${OPENSSL_ROOT}/providers/common/provider_err.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/ciphercommon.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/ciphercommon_block.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/ciphercommon_ccm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/ciphercommon_ccm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/ciphercommon_gcm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/ciphercommon_gcm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/ciphercommon_hw.c
    ${OPENSSL_ROOT}/providers/implementations/digests/digestcommon.c
    ${OPENSSL_ROOT}/ssl/record/tls_pad.c
)

set(LIBDEFAULT_COMMON_SOURCES
    ${OPENSSL_ROOT}/providers/common/bio_prov.c
    ${OPENSSL_ROOT}/providers/common/capabilities.c
    ${OPENSSL_ROOT}/providers/common/der/der_rsa_sig.c
    ${OPENSSL_ROOT}/providers/common/der/der_sm2_key.c
    ${OPENSSL_ROOT}/providers/common/der/der_sm2_sig.c
    ${OPENSSL_ROOT}/providers/common/digest_to_nid.c
    ${OPENSSL_ROOT}/providers/common/provider_seeding.c
    ${OPENSSL_ROOT}/providers/common/provider_util.c
    ${OPENSSL_ROOT}/providers/common/securitycheck.c
    ${OPENSSL_ROOT}/providers/common/securitycheck_default.c
    ${OPENSSL_ROOT}/providers/implementations/asymciphers/rsa_enc.c
    ${OPENSSL_ROOT}/providers/implementations/asymciphers/sm2_enc.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_cbc_hmac_sha.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_cbc_hmac_sha1_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_cbc_hmac_sha256_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_ccm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_ccm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_gcm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_gcm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_ocb.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_ocb_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_siv.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_siv_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_wrp.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_xts.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_xts_fips.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aes_xts_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aria.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aria_ccm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aria_ccm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aria_gcm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aria_gcm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_aria_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_camellia.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_camellia_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_chacha20.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_chacha20_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_chacha20_poly1305.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_chacha20_poly1305_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_cts.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_null.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_sm4.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_sm4_ccm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_sm4_ccm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_sm4_gcm.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_sm4_gcm_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_sm4_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_tdes.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_tdes_common.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_tdes_default.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_tdes_default_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_tdes_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_tdes_wrap.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_tdes_wrap_hw.c
    ${OPENSSL_ROOT}/providers/implementations/digests/blake2_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/blake2b_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/blake2s_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/md5_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/md5_sha1_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/null_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/ripemd_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/sha2_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/sha3_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/sm3_prov.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/decode_der2key.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/decode_epki2pki.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/decode_msblob2key.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/decode_pem2der.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/decode_pvk2key.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/decode_spki2typespki.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/encode_key2any.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/encode_key2blob.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/encode_key2ms.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/encode_key2text.c
    ${OPENSSL_ROOT}/providers/implementations/encode_decode/endecoder_common.c
    ${OPENSSL_ROOT}/providers/implementations/exchange/dh_exch.c
    ${OPENSSL_ROOT}/providers/implementations/exchange/ecdh_exch.c
    ${OPENSSL_ROOT}/providers/implementations/exchange/ecx_exch.c
    ${OPENSSL_ROOT}/providers/implementations/exchange/kdf_exch.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/hkdf.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/kbkdf.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/krb5kdf.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/pbkdf2.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/pbkdf2_fips.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/pkcs12kdf.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/scrypt.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/sshkdf.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/sskdf.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/tls1_prf.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/x942kdf.c
    ${OPENSSL_ROOT}/providers/implementations/kem/rsa_kem.c
    ${OPENSSL_ROOT}/providers/implementations/keymgmt/dh_kmgmt.c
    ${OPENSSL_ROOT}/providers/implementations/keymgmt/dsa_kmgmt.c
    ${OPENSSL_ROOT}/providers/implementations/keymgmt/ec_kmgmt.c
    ${OPENSSL_ROOT}/providers/implementations/keymgmt/ecx_kmgmt.c
    ${OPENSSL_ROOT}/providers/implementations/keymgmt/kdf_legacy_kmgmt.c
    ${OPENSSL_ROOT}/providers/implementations/keymgmt/mac_legacy_kmgmt.c
    ${OPENSSL_ROOT}/providers/implementations/keymgmt/rsa_kmgmt.c
    ${OPENSSL_ROOT}/providers/implementations/macs/blake2b_mac.c
    ${OPENSSL_ROOT}/providers/implementations/macs/blake2s_mac.c
    ${OPENSSL_ROOT}/providers/implementations/macs/cmac_prov.c
    ${OPENSSL_ROOT}/providers/implementations/macs/gmac_prov.c
    ${OPENSSL_ROOT}/providers/implementations/macs/hmac_prov.c
    ${OPENSSL_ROOT}/providers/implementations/macs/kmac_prov.c
    ${OPENSSL_ROOT}/providers/implementations/macs/poly1305_prov.c
    ${OPENSSL_ROOT}/providers/implementations/macs/siphash_prov.c
    ${OPENSSL_ROOT}/providers/implementations/rands/crngt.c
    ${OPENSSL_ROOT}/providers/implementations/rands/drbg.c
    ${OPENSSL_ROOT}/providers/implementations/rands/drbg_ctr.c
    ${OPENSSL_ROOT}/providers/implementations/rands/drbg_hash.c
    ${OPENSSL_ROOT}/providers/implementations/rands/drbg_hmac.c
    ${OPENSSL_ROOT}/providers/implementations/rands/seed_src.c
    ${OPENSSL_ROOT}/providers/implementations/rands/seeding/rand_cpu_x86.c
    ${OPENSSL_ROOT}/providers/implementations/rands/seeding/rand_tsc.c
    ${OPENSSL_ROOT}/providers/implementations/rands/seeding/rand_unix.c
    ${OPENSSL_ROOT}/providers/implementations/rands/seeding/rand_win.c
    ${OPENSSL_ROOT}/providers/implementations/rands/test_rng.c
    ${OPENSSL_ROOT}/providers/implementations/signature/dsa_sig.c
    ${OPENSSL_ROOT}/providers/implementations/signature/ecdsa_sig.c
    ${OPENSSL_ROOT}/providers/implementations/signature/eddsa_sig.c
    ${OPENSSL_ROOT}/providers/implementations/signature/mac_legacy_sig.c
    ${OPENSSL_ROOT}/providers/implementations/signature/rsa_sig.c
    ${OPENSSL_ROOT}/providers/implementations/signature/sm2_sig.c
    ${OPENSSL_ROOT}/providers/implementations/storemgmt/file_store.c
    ${OPENSSL_ROOT}/providers/implementations/storemgmt/file_store_any2obj.c
    ${OPENSSL_ROOT}/ssl/s3_cbc.c
)

set(LIBLEGACY_SOURCES
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_blowfish.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_blowfish_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_cast5.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_cast5_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_des.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_des_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_desx.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_desx_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_idea.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_idea_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_rc2.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_rc2_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_rc4.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_rc4_hmac_md5.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_rc4_hmac_md5_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_rc4_hw.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_seed.c
    ${OPENSSL_ROOT}/providers/implementations/ciphers/cipher_seed_hw.c
    ${OPENSSL_ROOT}/providers/implementations/digests/md4_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/mdc2_prov.c
    ${OPENSSL_ROOT}/providers/implementations/digests/wp_prov.c
    ${OPENSSL_ROOT}/providers/implementations/kdfs/pbkdf1.c
)

set(CRYPTO_SOURCES
    ${OPENSSL_ROOT}/crypto/aes/aes_cfb.c
    ${OPENSSL_ROOT}/crypto/aes/aes_ecb.c
    ${OPENSSL_ROOT}/crypto/aes/aes_ige.c
    ${OPENSSL_ROOT}/crypto/aes/aes_misc.c
    ${OPENSSL_ROOT}/crypto/aes/aes_ofb.c
    ${OPENSSL_ROOT}/crypto/aes/aes_wrap.c
    ${OPENSSL_ROOT}/crypto/aria/aria.c
    ${OPENSSL_ROOT}/crypto/asn1/a_bitstr.c
    ${OPENSSL_ROOT}/crypto/asn1/a_d2i_fp.c
    ${OPENSSL_ROOT}/crypto/asn1/a_digest.c
    ${OPENSSL_ROOT}/crypto/asn1/a_dup.c
    ${OPENSSL_ROOT}/crypto/asn1/a_gentm.c
    ${OPENSSL_ROOT}/crypto/asn1/a_i2d_fp.c
    ${OPENSSL_ROOT}/crypto/asn1/a_int.c
    ${OPENSSL_ROOT}/crypto/asn1/a_mbstr.c
    ${OPENSSL_ROOT}/crypto/asn1/a_object.c
    ${OPENSSL_ROOT}/crypto/asn1/a_octet.c
    ${OPENSSL_ROOT}/crypto/asn1/a_print.c
    ${OPENSSL_ROOT}/crypto/asn1/a_sign.c
    ${OPENSSL_ROOT}/crypto/asn1/a_strex.c
    ${OPENSSL_ROOT}/crypto/asn1/a_strnid.c
    ${OPENSSL_ROOT}/crypto/asn1/a_time.c
    ${OPENSSL_ROOT}/crypto/asn1/a_type.c
    ${OPENSSL_ROOT}/crypto/asn1/a_utctm.c
    ${OPENSSL_ROOT}/crypto/asn1/a_utf8.c
    ${OPENSSL_ROOT}/crypto/asn1/a_verify.c
    ${OPENSSL_ROOT}/crypto/asn1/ameth_lib.c
    ${OPENSSL_ROOT}/crypto/asn1/asn1_err.c
    ${OPENSSL_ROOT}/crypto/asn1/asn1_gen.c
    ${OPENSSL_ROOT}/crypto/asn1/asn1_item_list.c
    ${OPENSSL_ROOT}/crypto/asn1/asn1_lib.c
    ${OPENSSL_ROOT}/crypto/asn1/asn1_parse.c
    ${OPENSSL_ROOT}/crypto/asn1/asn_mime.c
    ${OPENSSL_ROOT}/crypto/asn1/asn_moid.c
    ${OPENSSL_ROOT}/crypto/asn1/asn_mstbl.c
    ${OPENSSL_ROOT}/crypto/asn1/asn_pack.c
    ${OPENSSL_ROOT}/crypto/asn1/bio_asn1.c
    ${OPENSSL_ROOT}/crypto/asn1/bio_ndef.c
    ${OPENSSL_ROOT}/crypto/asn1/d2i_param.c
    ${OPENSSL_ROOT}/crypto/asn1/d2i_pr.c
    ${OPENSSL_ROOT}/crypto/asn1/d2i_pu.c
    ${OPENSSL_ROOT}/crypto/asn1/evp_asn1.c
    ${OPENSSL_ROOT}/crypto/asn1/f_int.c
    ${OPENSSL_ROOT}/crypto/asn1/f_string.c
    ${OPENSSL_ROOT}/crypto/asn1/i2d_evp.c
    ${OPENSSL_ROOT}/crypto/asn1/n_pkey.c
    ${OPENSSL_ROOT}/crypto/asn1/nsseq.c
    ${OPENSSL_ROOT}/crypto/asn1/p5_pbe.c
    ${OPENSSL_ROOT}/crypto/asn1/p5_pbev2.c
    ${OPENSSL_ROOT}/crypto/asn1/p5_scrypt.c
    ${OPENSSL_ROOT}/crypto/asn1/p8_pkey.c
    ${OPENSSL_ROOT}/crypto/asn1/t_bitst.c
    ${OPENSSL_ROOT}/crypto/asn1/t_pkey.c
    ${OPENSSL_ROOT}/crypto/asn1/t_spki.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_dec.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_enc.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_fre.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_new.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_prn.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_scn.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_typ.c
    ${OPENSSL_ROOT}/crypto/asn1/tasn_utl.c
    ${OPENSSL_ROOT}/crypto/asn1/x_algor.c
    ${OPENSSL_ROOT}/crypto/asn1/x_bignum.c
    ${OPENSSL_ROOT}/crypto/asn1/x_info.c
    ${OPENSSL_ROOT}/crypto/asn1/x_int64.c
    ${OPENSSL_ROOT}/crypto/asn1/x_long.c
    ${OPENSSL_ROOT}/crypto/asn1/x_pkey.c
    ${OPENSSL_ROOT}/crypto/asn1/x_sig.c
    ${OPENSSL_ROOT}/crypto/asn1/x_spki.c
    ${OPENSSL_ROOT}/crypto/asn1/x_val.c
    ${OPENSSL_ROOT}/crypto/asn1_dsa.c
    ${OPENSSL_ROOT}/crypto/async/arch/async_null.c
    ${OPENSSL_ROOT}/crypto/async/arch/async_posix.c
    ${OPENSSL_ROOT}/crypto/async/arch/async_win.c
    ${OPENSSL_ROOT}/crypto/async/async.c
    ${OPENSSL_ROOT}/crypto/async/async_err.c
    ${OPENSSL_ROOT}/crypto/async/async_wait.c
    ${OPENSSL_ROOT}/crypto/bf/bf_cfb64.c
    ${OPENSSL_ROOT}/crypto/bf/bf_ecb.c
    ${OPENSSL_ROOT}/crypto/bf/bf_enc.c
    ${OPENSSL_ROOT}/crypto/bf/bf_ofb64.c
    ${OPENSSL_ROOT}/crypto/bf/bf_skey.c
    ${OPENSSL_ROOT}/crypto/bio/bf_buff.c
    ${OPENSSL_ROOT}/crypto/bio/bf_lbuf.c
    ${OPENSSL_ROOT}/crypto/bio/bf_nbio.c
    ${OPENSSL_ROOT}/crypto/bio/bf_null.c
    ${OPENSSL_ROOT}/crypto/bio/bf_prefix.c
    ${OPENSSL_ROOT}/crypto/bio/bf_readbuff.c
    ${OPENSSL_ROOT}/crypto/bio/bio_addr.c
    ${OPENSSL_ROOT}/crypto/bio/bio_cb.c
    ${OPENSSL_ROOT}/crypto/bio/bio_dump.c
    ${OPENSSL_ROOT}/crypto/bio/bio_err.c
    ${OPENSSL_ROOT}/crypto/bio/bio_lib.c
    ${OPENSSL_ROOT}/crypto/bio/bio_meth.c
    ${OPENSSL_ROOT}/crypto/bio/bio_print.c
    ${OPENSSL_ROOT}/crypto/bio/bio_sock.c
    ${OPENSSL_ROOT}/crypto/bio/bio_sock2.c
    ${OPENSSL_ROOT}/crypto/bio/bss_acpt.c
    ${OPENSSL_ROOT}/crypto/bio/bss_bio.c
    ${OPENSSL_ROOT}/crypto/bio/bss_conn.c
    ${OPENSSL_ROOT}/crypto/bio/bss_core.c
    ${OPENSSL_ROOT}/crypto/bio/bss_dgram.c
    ${OPENSSL_ROOT}/crypto/bio/bss_fd.c
    ${OPENSSL_ROOT}/crypto/bio/bss_file.c
    ${OPENSSL_ROOT}/crypto/bio/bss_log.c
    ${OPENSSL_ROOT}/crypto/bio/bss_mem.c
    ${OPENSSL_ROOT}/crypto/bio/bss_null.c
    ${OPENSSL_ROOT}/crypto/bio/bss_sock.c
    ${OPENSSL_ROOT}/crypto/bio/ossl_core_bio.c
    ${OPENSSL_ROOT}/crypto/bn/bn_add.c
    ${OPENSSL_ROOT}/crypto/bn/bn_blind.c
    ${OPENSSL_ROOT}/crypto/bn/bn_const.c
    ${OPENSSL_ROOT}/crypto/bn/bn_conv.c
    ${OPENSSL_ROOT}/crypto/bn/bn_ctx.c
    ${OPENSSL_ROOT}/crypto/bn/bn_depr.c
    ${OPENSSL_ROOT}/crypto/bn/bn_dh.c
    ${OPENSSL_ROOT}/crypto/bn/bn_div.c
    ${OPENSSL_ROOT}/crypto/bn/bn_err.c
    ${OPENSSL_ROOT}/crypto/bn/bn_exp.c
    ${OPENSSL_ROOT}/crypto/bn/bn_exp2.c
    ${OPENSSL_ROOT}/crypto/bn/bn_gcd.c
    ${OPENSSL_ROOT}/crypto/bn/bn_gf2m.c
    ${OPENSSL_ROOT}/crypto/bn/bn_intern.c
    ${OPENSSL_ROOT}/crypto/bn/bn_kron.c
    ${OPENSSL_ROOT}/crypto/bn/bn_lib.c
    ${OPENSSL_ROOT}/crypto/bn/bn_mod.c
    ${OPENSSL_ROOT}/crypto/bn/bn_mont.c
    ${OPENSSL_ROOT}/crypto/bn/bn_mpi.c
    ${OPENSSL_ROOT}/crypto/bn/bn_mul.c
    ${OPENSSL_ROOT}/crypto/bn/bn_nist.c
    ${OPENSSL_ROOT}/crypto/bn/bn_prime.c
    ${OPENSSL_ROOT}/crypto/bn/bn_print.c
    ${OPENSSL_ROOT}/crypto/bn/bn_rand.c
    ${OPENSSL_ROOT}/crypto/bn/bn_recp.c
    ${OPENSSL_ROOT}/crypto/bn/bn_rsa_fips186_4.c
    ${OPENSSL_ROOT}/crypto/bn/bn_shift.c
    ${OPENSSL_ROOT}/crypto/bn/bn_sqr.c
    ${OPENSSL_ROOT}/crypto/bn/bn_sqrt.c
    ${OPENSSL_ROOT}/crypto/bn/bn_srp.c
    ${OPENSSL_ROOT}/crypto/bn/bn_word.c
    ${OPENSSL_ROOT}/crypto/bn/bn_x931p.c
    ${OPENSSL_ROOT}/crypto/bsearch.c
    ${OPENSSL_ROOT}/crypto/buffer/buf_err.c
    ${OPENSSL_ROOT}/crypto/buffer/buffer.c
    ${OPENSSL_ROOT}/crypto/camellia/cmll_cfb.c
    ${OPENSSL_ROOT}/crypto/camellia/cmll_ctr.c
    ${OPENSSL_ROOT}/crypto/camellia/cmll_ecb.c
    ${OPENSSL_ROOT}/crypto/camellia/cmll_misc.c
    ${OPENSSL_ROOT}/crypto/camellia/cmll_ofb.c
    ${OPENSSL_ROOT}/crypto/cast/c_cfb64.c
    ${OPENSSL_ROOT}/crypto/cast/c_ecb.c
    ${OPENSSL_ROOT}/crypto/cast/c_enc.c
    ${OPENSSL_ROOT}/crypto/cast/c_ofb64.c
    ${OPENSSL_ROOT}/crypto/cast/c_skey.c
    ${OPENSSL_ROOT}/crypto/cmac/cmac.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_asn.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_client.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_ctx.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_err.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_hdr.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_http.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_msg.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_protect.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_server.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_status.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_util.c
    ${OPENSSL_ROOT}/crypto/cmp/cmp_vfy.c
    ${OPENSSL_ROOT}/crypto/cms/cms_asn1.c
    ${OPENSSL_ROOT}/crypto/cms/cms_att.c
    ${OPENSSL_ROOT}/crypto/cms/cms_cd.c
    ${OPENSSL_ROOT}/crypto/cms/cms_dd.c
    ${OPENSSL_ROOT}/crypto/cms/cms_dh.c
    ${OPENSSL_ROOT}/crypto/cms/cms_ec.c
    ${OPENSSL_ROOT}/crypto/cms/cms_enc.c
    ${OPENSSL_ROOT}/crypto/cms/cms_env.c
    ${OPENSSL_ROOT}/crypto/cms/cms_err.c
    ${OPENSSL_ROOT}/crypto/cms/cms_ess.c
    ${OPENSSL_ROOT}/crypto/cms/cms_io.c
    ${OPENSSL_ROOT}/crypto/cms/cms_kari.c
    ${OPENSSL_ROOT}/crypto/cms/cms_lib.c
    ${OPENSSL_ROOT}/crypto/cms/cms_pwri.c
    ${OPENSSL_ROOT}/crypto/cms/cms_rsa.c
    ${OPENSSL_ROOT}/crypto/cms/cms_sd.c
    ${OPENSSL_ROOT}/crypto/cms/cms_smime.c
    ${OPENSSL_ROOT}/crypto/comp/c_zlib.c
    ${OPENSSL_ROOT}/crypto/comp/comp_err.c
    ${OPENSSL_ROOT}/crypto/comp/comp_lib.c
    ${OPENSSL_ROOT}/crypto/conf/conf_api.c
    ${OPENSSL_ROOT}/crypto/conf/conf_def.c
    ${OPENSSL_ROOT}/crypto/conf/conf_err.c
    ${OPENSSL_ROOT}/crypto/conf/conf_lib.c
    ${OPENSSL_ROOT}/crypto/conf/conf_mall.c
    ${OPENSSL_ROOT}/crypto/conf/conf_mod.c
    ${OPENSSL_ROOT}/crypto/conf/conf_sap.c
    ${OPENSSL_ROOT}/crypto/conf/conf_ssl.c
    ${OPENSSL_ROOT}/crypto/context.c
    ${OPENSSL_ROOT}/crypto/core_algorithm.c
    ${OPENSSL_ROOT}/crypto/core_fetch.c
    ${OPENSSL_ROOT}/crypto/core_namemap.c
    ${OPENSSL_ROOT}/crypto/cpt_err.c
    ${OPENSSL_ROOT}/crypto/cpuid.c
    ${OPENSSL_ROOT}/crypto/crmf/crmf_asn.c
    ${OPENSSL_ROOT}/crypto/crmf/crmf_err.c
    ${OPENSSL_ROOT}/crypto/crmf/crmf_lib.c
    ${OPENSSL_ROOT}/crypto/crmf/crmf_pbm.c
    ${OPENSSL_ROOT}/crypto/cryptlib.c
    ${OPENSSL_ROOT}/crypto/ct/ct_b64.c
    ${OPENSSL_ROOT}/crypto/ct/ct_err.c
    ${OPENSSL_ROOT}/crypto/ct/ct_log.c
    ${OPENSSL_ROOT}/crypto/ct/ct_oct.c
    ${OPENSSL_ROOT}/crypto/ct/ct_policy.c
    ${OPENSSL_ROOT}/crypto/ct/ct_prn.c
    ${OPENSSL_ROOT}/crypto/ct/ct_sct.c
    ${OPENSSL_ROOT}/crypto/ct/ct_sct_ctx.c
    ${OPENSSL_ROOT}/crypto/ct/ct_vfy.c
    ${OPENSSL_ROOT}/crypto/ct/ct_x509v3.c
    ${OPENSSL_ROOT}/crypto/ctype.c
    ${OPENSSL_ROOT}/crypto/cversion.c
    ${OPENSSL_ROOT}/crypto/der_writer.c
    ${OPENSSL_ROOT}/crypto/des/cbc_cksm.c
    ${OPENSSL_ROOT}/crypto/des/cbc_enc.c
    ${OPENSSL_ROOT}/crypto/des/cfb64ede.c
    ${OPENSSL_ROOT}/crypto/des/cfb64enc.c
    ${OPENSSL_ROOT}/crypto/des/cfb_enc.c
    ${OPENSSL_ROOT}/crypto/des/des_enc.c
    ${OPENSSL_ROOT}/crypto/des/ecb3_enc.c
    ${OPENSSL_ROOT}/crypto/des/ecb_enc.c
    ${OPENSSL_ROOT}/crypto/des/fcrypt.c
    ${OPENSSL_ROOT}/crypto/des/fcrypt_b.c
    ${OPENSSL_ROOT}/crypto/des/ofb64ede.c
    ${OPENSSL_ROOT}/crypto/des/ofb64enc.c
    ${OPENSSL_ROOT}/crypto/des/ofb_enc.c
    ${OPENSSL_ROOT}/crypto/des/pcbc_enc.c
    ${OPENSSL_ROOT}/crypto/des/qud_cksm.c
    ${OPENSSL_ROOT}/crypto/des/rand_key.c
    ${OPENSSL_ROOT}/crypto/des/set_key.c
    ${OPENSSL_ROOT}/crypto/des/str2key.c
    ${OPENSSL_ROOT}/crypto/des/xcbc_enc.c
    ${OPENSSL_ROOT}/crypto/dh/dh_ameth.c
    ${OPENSSL_ROOT}/crypto/dh/dh_asn1.c
    ${OPENSSL_ROOT}/crypto/dh/dh_backend.c
    ${OPENSSL_ROOT}/crypto/dh/dh_check.c
    ${OPENSSL_ROOT}/crypto/dh/dh_depr.c
    ${OPENSSL_ROOT}/crypto/dh/dh_err.c
    ${OPENSSL_ROOT}/crypto/dh/dh_gen.c
    ${OPENSSL_ROOT}/crypto/dh/dh_group_params.c
    ${OPENSSL_ROOT}/crypto/dh/dh_kdf.c
    ${OPENSSL_ROOT}/crypto/dh/dh_key.c
    ${OPENSSL_ROOT}/crypto/dh/dh_lib.c
    ${OPENSSL_ROOT}/crypto/dh/dh_meth.c
    ${OPENSSL_ROOT}/crypto/dh/dh_pmeth.c
    ${OPENSSL_ROOT}/crypto/dh/dh_prn.c
    ${OPENSSL_ROOT}/crypto/dh/dh_rfc5114.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_ameth.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_asn1.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_backend.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_check.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_depr.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_err.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_gen.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_key.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_lib.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_meth.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_ossl.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_pmeth.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_prn.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_sign.c
    ${OPENSSL_ROOT}/crypto/dsa/dsa_vrf.c
    ${OPENSSL_ROOT}/crypto/dso/dso_dl.c
    ${OPENSSL_ROOT}/crypto/dso/dso_dlfcn.c
    ${OPENSSL_ROOT}/crypto/dso/dso_err.c
    ${OPENSSL_ROOT}/crypto/dso/dso_lib.c
    ${OPENSSL_ROOT}/crypto/dso/dso_openssl.c
    ${OPENSSL_ROOT}/crypto/dso/dso_vms.c
    ${OPENSSL_ROOT}/crypto/dso/dso_win32.c
    ${OPENSSL_ROOT}/crypto/ebcdic.c
    ${OPENSSL_ROOT}/crypto/ec/curve25519.c
    ${OPENSSL_ROOT}/crypto/ec/curve448/arch_32/f_impl32.c
    ${OPENSSL_ROOT}/crypto/ec/curve448/arch_64/f_impl64.c
    ${OPENSSL_ROOT}/crypto/ec/curve448/curve448.c
    ${OPENSSL_ROOT}/crypto/ec/curve448/curve448_tables.c
    ${OPENSSL_ROOT}/crypto/ec/curve448/eddsa.c
    ${OPENSSL_ROOT}/crypto/ec/curve448/f_generic.c
    ${OPENSSL_ROOT}/crypto/ec/curve448/scalar.c
    ${OPENSSL_ROOT}/crypto/ec/ec2_oct.c
    ${OPENSSL_ROOT}/crypto/ec/ec2_smpl.c
    ${OPENSSL_ROOT}/crypto/ec/ec_ameth.c
    ${OPENSSL_ROOT}/crypto/ec/ec_asn1.c
    ${OPENSSL_ROOT}/crypto/ec/ec_backend.c
    ${OPENSSL_ROOT}/crypto/ec/ec_check.c
    ${OPENSSL_ROOT}/crypto/ec/ec_curve.c
    ${OPENSSL_ROOT}/crypto/ec/ec_cvt.c
    ${OPENSSL_ROOT}/crypto/ec/ec_deprecated.c
    ${OPENSSL_ROOT}/crypto/ec/ec_err.c
    ${OPENSSL_ROOT}/crypto/ec/ec_key.c
    ${OPENSSL_ROOT}/crypto/ec/ec_kmeth.c
    ${OPENSSL_ROOT}/crypto/ec/ec_lib.c
    ${OPENSSL_ROOT}/crypto/ec/ec_mult.c
    ${OPENSSL_ROOT}/crypto/ec/ec_oct.c
    ${OPENSSL_ROOT}/crypto/ec/ec_pmeth.c
    ${OPENSSL_ROOT}/crypto/ec/ec_print.c
    ${OPENSSL_ROOT}/crypto/ec/ecdh_kdf.c
    ${OPENSSL_ROOT}/crypto/ec/ecdh_ossl.c
    ${OPENSSL_ROOT}/crypto/ec/ecdsa_ossl.c
    ${OPENSSL_ROOT}/crypto/ec/ecdsa_sign.c
    ${OPENSSL_ROOT}/crypto/ec/ecdsa_vrf.c
    ${OPENSSL_ROOT}/crypto/ec/eck_prn.c
    ${OPENSSL_ROOT}/crypto/ec/ecp_mont.c
    ${OPENSSL_ROOT}/crypto/ec/ecp_nist.c
    ${OPENSSL_ROOT}/crypto/ec/ecp_nistz256.c
    ${OPENSSL_ROOT}/crypto/ec/ecp_oct.c
    ${OPENSSL_ROOT}/crypto/ec/ecp_smpl.c
    ${OPENSSL_ROOT}/crypto/ec/ecx_backend.c
    ${OPENSSL_ROOT}/crypto/ec/ecx_key.c
    ${OPENSSL_ROOT}/crypto/ec/ecx_meth.c
    ${OPENSSL_ROOT}/crypto/encode_decode/decoder_err.c
    ${OPENSSL_ROOT}/crypto/encode_decode/decoder_lib.c
    ${OPENSSL_ROOT}/crypto/encode_decode/decoder_meth.c
    ${OPENSSL_ROOT}/crypto/encode_decode/decoder_pkey.c
    ${OPENSSL_ROOT}/crypto/encode_decode/encoder_err.c
    ${OPENSSL_ROOT}/crypto/encode_decode/encoder_lib.c
    ${OPENSSL_ROOT}/crypto/encode_decode/encoder_meth.c
    ${OPENSSL_ROOT}/crypto/encode_decode/encoder_pkey.c
    ${OPENSSL_ROOT}/crypto/engine/eng_all.c
    ${OPENSSL_ROOT}/crypto/engine/eng_cnf.c
    ${OPENSSL_ROOT}/crypto/engine/eng_ctrl.c
    ${OPENSSL_ROOT}/crypto/engine/eng_dyn.c
    ${OPENSSL_ROOT}/crypto/engine/eng_err.c
    ${OPENSSL_ROOT}/crypto/engine/eng_fat.c
    ${OPENSSL_ROOT}/crypto/engine/eng_init.c
    ${OPENSSL_ROOT}/crypto/engine/eng_lib.c
    ${OPENSSL_ROOT}/crypto/engine/eng_list.c
    ${OPENSSL_ROOT}/crypto/engine/eng_openssl.c
    ${OPENSSL_ROOT}/crypto/engine/eng_pkey.c
    ${OPENSSL_ROOT}/crypto/engine/eng_rdrand.c
    ${OPENSSL_ROOT}/crypto/engine/eng_table.c
    ${OPENSSL_ROOT}/crypto/engine/tb_asnmth.c
    ${OPENSSL_ROOT}/crypto/engine/tb_cipher.c
    ${OPENSSL_ROOT}/crypto/engine/tb_dh.c
    ${OPENSSL_ROOT}/crypto/engine/tb_digest.c
    ${OPENSSL_ROOT}/crypto/engine/tb_dsa.c
    ${OPENSSL_ROOT}/crypto/engine/tb_eckey.c
    ${OPENSSL_ROOT}/crypto/engine/tb_pkmeth.c
    ${OPENSSL_ROOT}/crypto/engine/tb_rand.c
    ${OPENSSL_ROOT}/crypto/engine/tb_rsa.c
    ${OPENSSL_ROOT}/crypto/err/err.c
    ${OPENSSL_ROOT}/crypto/err/err_all.c
    ${OPENSSL_ROOT}/crypto/err/err_all_legacy.c
    ${OPENSSL_ROOT}/crypto/err/err_blocks.c
    ${OPENSSL_ROOT}/crypto/err/err_prn.c
    ${OPENSSL_ROOT}/crypto/ess/ess_asn1.c
    ${OPENSSL_ROOT}/crypto/ess/ess_err.c
    ${OPENSSL_ROOT}/crypto/ess/ess_lib.c
    ${OPENSSL_ROOT}/crypto/evp/asymcipher.c
    ${OPENSSL_ROOT}/crypto/evp/bio_b64.c
    ${OPENSSL_ROOT}/crypto/evp/bio_enc.c
    ${OPENSSL_ROOT}/crypto/evp/bio_md.c
    ${OPENSSL_ROOT}/crypto/evp/bio_ok.c
    ${OPENSSL_ROOT}/crypto/evp/c_allc.c
    ${OPENSSL_ROOT}/crypto/evp/c_alld.c
    ${OPENSSL_ROOT}/crypto/evp/cmeth_lib.c
    ${OPENSSL_ROOT}/crypto/evp/ctrl_params_translate.c
    ${OPENSSL_ROOT}/crypto/evp/dh_ctrl.c
    ${OPENSSL_ROOT}/crypto/evp/dh_support.c
    ${OPENSSL_ROOT}/crypto/evp/digest.c
    ${OPENSSL_ROOT}/crypto/evp/dsa_ctrl.c
    ${OPENSSL_ROOT}/crypto/evp/e_aes.c
    ${OPENSSL_ROOT}/crypto/evp/e_aes_cbc_hmac_sha1.c
    ${OPENSSL_ROOT}/crypto/evp/e_aes_cbc_hmac_sha256.c
    ${OPENSSL_ROOT}/crypto/evp/e_aria.c
    ${OPENSSL_ROOT}/crypto/evp/e_bf.c
    ${OPENSSL_ROOT}/crypto/evp/e_camellia.c
    ${OPENSSL_ROOT}/crypto/evp/e_cast.c
    ${OPENSSL_ROOT}/crypto/evp/e_chacha20_poly1305.c
    ${OPENSSL_ROOT}/crypto/evp/e_des.c
    ${OPENSSL_ROOT}/crypto/evp/e_des3.c
    ${OPENSSL_ROOT}/crypto/evp/e_idea.c
    ${OPENSSL_ROOT}/crypto/evp/e_null.c
    ${OPENSSL_ROOT}/crypto/evp/e_old.c
    ${OPENSSL_ROOT}/crypto/evp/e_rc2.c
    ${OPENSSL_ROOT}/crypto/evp/e_rc4.c
    ${OPENSSL_ROOT}/crypto/evp/e_rc4_hmac_md5.c
    ${OPENSSL_ROOT}/crypto/evp/e_rc5.c
    ${OPENSSL_ROOT}/crypto/evp/e_seed.c
    ${OPENSSL_ROOT}/crypto/evp/e_sm4.c
    ${OPENSSL_ROOT}/crypto/evp/e_xcbc_d.c
    ${OPENSSL_ROOT}/crypto/evp/ec_ctrl.c
    ${OPENSSL_ROOT}/crypto/evp/ec_support.c
    ${OPENSSL_ROOT}/crypto/evp/encode.c
    ${OPENSSL_ROOT}/crypto/evp/evp_cnf.c
    ${OPENSSL_ROOT}/crypto/evp/evp_enc.c
    ${OPENSSL_ROOT}/crypto/evp/evp_err.c
    ${OPENSSL_ROOT}/crypto/evp/evp_fetch.c
    ${OPENSSL_ROOT}/crypto/evp/evp_key.c
    ${OPENSSL_ROOT}/crypto/evp/evp_lib.c
    ${OPENSSL_ROOT}/crypto/evp/evp_pbe.c
    ${OPENSSL_ROOT}/crypto/evp/evp_pkey.c
    ${OPENSSL_ROOT}/crypto/evp/evp_rand.c
    ${OPENSSL_ROOT}/crypto/evp/evp_utils.c
    ${OPENSSL_ROOT}/crypto/evp/exchange.c
    ${OPENSSL_ROOT}/crypto/evp/kdf_lib.c
    ${OPENSSL_ROOT}/crypto/evp/kdf_meth.c
    ${OPENSSL_ROOT}/crypto/evp/kem.c
    ${OPENSSL_ROOT}/crypto/evp/keymgmt_lib.c
    ${OPENSSL_ROOT}/crypto/evp/keymgmt_meth.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_blake2.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_md4.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_md5.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_md5_sha1.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_mdc2.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_ripemd.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_sha.c
    ${OPENSSL_ROOT}/crypto/evp/legacy_wp.c
    ${OPENSSL_ROOT}/crypto/evp/m_null.c
    ${OPENSSL_ROOT}/crypto/evp/m_sigver.c
    ${OPENSSL_ROOT}/crypto/evp/mac_lib.c
    ${OPENSSL_ROOT}/crypto/evp/mac_meth.c
    ${OPENSSL_ROOT}/crypto/evp/names.c
    ${OPENSSL_ROOT}/crypto/evp/p5_crpt.c
    ${OPENSSL_ROOT}/crypto/evp/p5_crpt2.c
    ${OPENSSL_ROOT}/crypto/evp/p_dec.c
    ${OPENSSL_ROOT}/crypto/evp/p_enc.c
    ${OPENSSL_ROOT}/crypto/evp/p_legacy.c
    ${OPENSSL_ROOT}/crypto/evp/p_lib.c
    ${OPENSSL_ROOT}/crypto/evp/p_open.c
    ${OPENSSL_ROOT}/crypto/evp/p_seal.c
    ${OPENSSL_ROOT}/crypto/evp/p_sign.c
    ${OPENSSL_ROOT}/crypto/evp/p_verify.c
    ${OPENSSL_ROOT}/crypto/evp/pbe_scrypt.c
    ${OPENSSL_ROOT}/crypto/evp/pmeth_check.c
    ${OPENSSL_ROOT}/crypto/evp/pmeth_gn.c
    ${OPENSSL_ROOT}/crypto/evp/pmeth_lib.c
    ${OPENSSL_ROOT}/crypto/evp/signature.c
    ${OPENSSL_ROOT}/crypto/ex_data.c
    ${OPENSSL_ROOT}/crypto/ffc/ffc_backend.c
    ${OPENSSL_ROOT}/crypto/ffc/ffc_dh.c
    ${OPENSSL_ROOT}/crypto/ffc/ffc_key_generate.c
    ${OPENSSL_ROOT}/crypto/ffc/ffc_key_validate.c
    ${OPENSSL_ROOT}/crypto/ffc/ffc_params.c
    ${OPENSSL_ROOT}/crypto/ffc/ffc_params_generate.c
    ${OPENSSL_ROOT}/crypto/ffc/ffc_params_validate.c
    ${OPENSSL_ROOT}/crypto/getenv.c
    ${OPENSSL_ROOT}/crypto/hmac/hmac.c
    ${OPENSSL_ROOT}/crypto/http/http_client.c
    ${OPENSSL_ROOT}/crypto/http/http_err.c
    ${OPENSSL_ROOT}/crypto/http/http_lib.c
    ${OPENSSL_ROOT}/crypto/idea/i_cbc.c
    ${OPENSSL_ROOT}/crypto/idea/i_cfb64.c
    ${OPENSSL_ROOT}/crypto/idea/i_ecb.c
    ${OPENSSL_ROOT}/crypto/idea/i_ofb64.c
    ${OPENSSL_ROOT}/crypto/idea/i_skey.c
    ${OPENSSL_ROOT}/crypto/info.c
    ${OPENSSL_ROOT}/crypto/init.c
    ${OPENSSL_ROOT}/crypto/initthread.c
    ${OPENSSL_ROOT}/crypto/kdf/kdf_err.c
    ${OPENSSL_ROOT}/crypto/lhash/lh_stats.c
    ${OPENSSL_ROOT}/crypto/lhash/lhash.c
    ${OPENSSL_ROOT}/crypto/md4/md4_dgst.c
    ${OPENSSL_ROOT}/crypto/md4/md4_one.c
    ${OPENSSL_ROOT}/crypto/md5/md5_dgst.c
    ${OPENSSL_ROOT}/crypto/md5/md5_one.c
    ${OPENSSL_ROOT}/crypto/md5/md5_sha1.c
    ${OPENSSL_ROOT}/crypto/mdc2/mdc2_one.c
    ${OPENSSL_ROOT}/crypto/mdc2/mdc2dgst.c
    ${OPENSSL_ROOT}/crypto/mem.c
    ${OPENSSL_ROOT}/crypto/mem_sec.c
    ${OPENSSL_ROOT}/crypto/modes/cbc128.c
    ${OPENSSL_ROOT}/crypto/modes/ccm128.c
    ${OPENSSL_ROOT}/crypto/modes/cfb128.c
    ${OPENSSL_ROOT}/crypto/modes/ctr128.c
    ${OPENSSL_ROOT}/crypto/modes/cts128.c
    ${OPENSSL_ROOT}/crypto/modes/gcm128.c
    ${OPENSSL_ROOT}/crypto/modes/ocb128.c
    ${OPENSSL_ROOT}/crypto/modes/ofb128.c
    ${OPENSSL_ROOT}/crypto/modes/siv128.c
    ${OPENSSL_ROOT}/crypto/modes/wrap128.c
    ${OPENSSL_ROOT}/crypto/modes/xts128.c
    ${OPENSSL_ROOT}/crypto/o_dir.c
    ${OPENSSL_ROOT}/crypto/o_fopen.c
    ${OPENSSL_ROOT}/crypto/o_init.c
    ${OPENSSL_ROOT}/crypto/o_str.c
    ${OPENSSL_ROOT}/crypto/o_time.c
    ${OPENSSL_ROOT}/crypto/objects/o_names.c
    ${OPENSSL_ROOT}/crypto/objects/obj_dat.c
    ${OPENSSL_ROOT}/crypto/objects/obj_err.c
    ${OPENSSL_ROOT}/crypto/objects/obj_lib.c
    ${OPENSSL_ROOT}/crypto/objects/obj_xref.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_asn.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_cl.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_err.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_ext.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_http.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_lib.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_prn.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_srv.c
    ${OPENSSL_ROOT}/crypto/ocsp/ocsp_vfy.c
    ${OPENSSL_ROOT}/crypto/ocsp/v3_ocsp.c
    ${OPENSSL_ROOT}/crypto/packet.c
    ${OPENSSL_ROOT}/crypto/param_build.c
    ${OPENSSL_ROOT}/crypto/param_build_set.c
    ${OPENSSL_ROOT}/crypto/params.c
    ${OPENSSL_ROOT}/crypto/params_dup.c
    ${OPENSSL_ROOT}/crypto/params_from_text.c
    ${OPENSSL_ROOT}/crypto/passphrase.c
    ${OPENSSL_ROOT}/crypto/pem/pem_all.c
    ${OPENSSL_ROOT}/crypto/pem/pem_err.c
    ${OPENSSL_ROOT}/crypto/pem/pem_info.c
    ${OPENSSL_ROOT}/crypto/pem/pem_lib.c
    ${OPENSSL_ROOT}/crypto/pem/pem_oth.c
    ${OPENSSL_ROOT}/crypto/pem/pem_pk8.c
    ${OPENSSL_ROOT}/crypto/pem/pem_pkey.c
    ${OPENSSL_ROOT}/crypto/pem/pem_sign.c
    ${OPENSSL_ROOT}/crypto/pem/pem_x509.c
    ${OPENSSL_ROOT}/crypto/pem/pem_xaux.c
    ${OPENSSL_ROOT}/crypto/pem/pvkfmt.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_add.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_asn.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_attr.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_crpt.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_crt.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_decr.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_init.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_key.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_kiss.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_mutl.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_npas.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_p8d.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_p8e.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_sbag.c
    ${OPENSSL_ROOT}/crypto/pkcs12/p12_utl.c
    ${OPENSSL_ROOT}/crypto/pkcs12/pk12err.c
    ${OPENSSL_ROOT}/crypto/pkcs7/bio_pk7.c
    ${OPENSSL_ROOT}/crypto/pkcs7/pk7_asn1.c
    ${OPENSSL_ROOT}/crypto/pkcs7/pk7_attr.c
    ${OPENSSL_ROOT}/crypto/pkcs7/pk7_doit.c
    ${OPENSSL_ROOT}/crypto/pkcs7/pk7_lib.c
    ${OPENSSL_ROOT}/crypto/pkcs7/pk7_mime.c
    ${OPENSSL_ROOT}/crypto/pkcs7/pk7_smime.c
    ${OPENSSL_ROOT}/crypto/pkcs7/pkcs7err.c
    ${OPENSSL_ROOT}/crypto/poly1305/poly1305.c
    ${OPENSSL_ROOT}/crypto/property/defn_cache.c
    ${OPENSSL_ROOT}/crypto/property/property.c
    ${OPENSSL_ROOT}/crypto/property/property_err.c
    ${OPENSSL_ROOT}/crypto/property/property_parse.c
    ${OPENSSL_ROOT}/crypto/property/property_query.c
    ${OPENSSL_ROOT}/crypto/property/property_string.c
    ${OPENSSL_ROOT}/crypto/provider.c
    ${OPENSSL_ROOT}/crypto/provider_child.c
    ${OPENSSL_ROOT}/crypto/provider_conf.c
    ${OPENSSL_ROOT}/crypto/provider_core.c
    ${OPENSSL_ROOT}/crypto/provider_predefined.c
    ${OPENSSL_ROOT}/crypto/punycode.c
    ${OPENSSL_ROOT}/crypto/rand/prov_seed.c
    ${OPENSSL_ROOT}/crypto/rand/rand_deprecated.c
    ${OPENSSL_ROOT}/crypto/rand/rand_err.c
    ${OPENSSL_ROOT}/crypto/rand/rand_lib.c
    ${OPENSSL_ROOT}/crypto/rand/rand_meth.c
    ${OPENSSL_ROOT}/crypto/rand/rand_pool.c
    ${OPENSSL_ROOT}/crypto/rand/randfile.c
    ${OPENSSL_ROOT}/crypto/rc2/rc2_cbc.c
    ${OPENSSL_ROOT}/crypto/rc2/rc2_ecb.c
    ${OPENSSL_ROOT}/crypto/rc2/rc2_skey.c
    ${OPENSSL_ROOT}/crypto/rc2/rc2cfb64.c
    ${OPENSSL_ROOT}/crypto/rc2/rc2ofb64.c
    ${OPENSSL_ROOT}/crypto/ripemd/rmd_dgst.c
    ${OPENSSL_ROOT}/crypto/ripemd/rmd_one.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_ameth.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_asn1.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_backend.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_chk.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_crpt.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_depr.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_err.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_gen.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_lib.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_meth.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_mp.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_mp_names.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_none.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_oaep.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_ossl.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_pk1.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_pmeth.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_prn.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_pss.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_saos.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_schemes.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_sign.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_sp800_56b_check.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_sp800_56b_gen.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_x931.c
    ${OPENSSL_ROOT}/crypto/rsa/rsa_x931g.c
    ${OPENSSL_ROOT}/crypto/seed/seed.c
    ${OPENSSL_ROOT}/crypto/seed/seed_cbc.c
    ${OPENSSL_ROOT}/crypto/seed/seed_cfb.c
    ${OPENSSL_ROOT}/crypto/seed/seed_ecb.c
    ${OPENSSL_ROOT}/crypto/seed/seed_ofb.c
    ${OPENSSL_ROOT}/crypto/self_test_core.c
    ${OPENSSL_ROOT}/crypto/sha/sha1_one.c
    ${OPENSSL_ROOT}/crypto/sha/sha1dgst.c
    ${OPENSSL_ROOT}/crypto/sha/sha256.c
    ${OPENSSL_ROOT}/crypto/sha/sha3.c
    ${OPENSSL_ROOT}/crypto/sha/sha512.c
    ${OPENSSL_ROOT}/crypto/siphash/siphash.c
    ${OPENSSL_ROOT}/crypto/sm2/sm2_crypt.c
    ${OPENSSL_ROOT}/crypto/sm2/sm2_err.c
    ${OPENSSL_ROOT}/crypto/sm2/sm2_key.c
    ${OPENSSL_ROOT}/crypto/sm2/sm2_sign.c
    ${OPENSSL_ROOT}/crypto/sm3/legacy_sm3.c
    ${OPENSSL_ROOT}/crypto/sm3/sm3.c
    ${OPENSSL_ROOT}/crypto/sm4/sm4.c
    ${OPENSSL_ROOT}/crypto/sparse_array.c
    ${OPENSSL_ROOT}/crypto/srp/srp_lib.c
    ${OPENSSL_ROOT}/crypto/srp/srp_vfy.c
    ${OPENSSL_ROOT}/crypto/stack/stack.c
    ${OPENSSL_ROOT}/crypto/store/store_err.c
    ${OPENSSL_ROOT}/crypto/store/store_init.c
    ${OPENSSL_ROOT}/crypto/store/store_lib.c
    ${OPENSSL_ROOT}/crypto/store/store_meth.c
    ${OPENSSL_ROOT}/crypto/store/store_register.c
    ${OPENSSL_ROOT}/crypto/store/store_result.c
    ${OPENSSL_ROOT}/crypto/store/store_strings.c
    ${OPENSSL_ROOT}/crypto/threads_lib.c
    ${OPENSSL_ROOT}/crypto/threads_none.c
    ${OPENSSL_ROOT}/crypto/threads_pthread.c
    ${OPENSSL_ROOT}/crypto/threads_win.c
    ${OPENSSL_ROOT}/crypto/trace.c
    ${OPENSSL_ROOT}/crypto/ts/ts_asn1.c
    ${OPENSSL_ROOT}/crypto/ts/ts_conf.c
    ${OPENSSL_ROOT}/crypto/ts/ts_err.c
    ${OPENSSL_ROOT}/crypto/ts/ts_lib.c
    ${OPENSSL_ROOT}/crypto/ts/ts_req_print.c
    ${OPENSSL_ROOT}/crypto/ts/ts_req_utils.c
    ${OPENSSL_ROOT}/crypto/ts/ts_rsp_print.c
    ${OPENSSL_ROOT}/crypto/ts/ts_rsp_sign.c
    ${OPENSSL_ROOT}/crypto/ts/ts_rsp_utils.c
    ${OPENSSL_ROOT}/crypto/ts/ts_rsp_verify.c
    ${OPENSSL_ROOT}/crypto/ts/ts_verify_ctx.c
    ${OPENSSL_ROOT}/crypto/txt_db/txt_db.c
    ${OPENSSL_ROOT}/crypto/ui/ui_err.c
    ${OPENSSL_ROOT}/crypto/ui/ui_lib.c
    ${OPENSSL_ROOT}/crypto/ui/ui_null.c
    ${OPENSSL_ROOT}/crypto/ui/ui_openssl.c
    ${OPENSSL_ROOT}/crypto/ui/ui_util.c
    ${OPENSSL_ROOT}/crypto/uid.c
    ${OPENSSL_ROOT}/crypto/whrlpool/wp_dgst.c
    ${OPENSSL_ROOT}/crypto/x509/by_dir.c
    ${OPENSSL_ROOT}/crypto/x509/by_file.c
    ${OPENSSL_ROOT}/crypto/x509/by_store.c
    ${OPENSSL_ROOT}/crypto/x509/pcy_cache.c
    ${OPENSSL_ROOT}/crypto/x509/pcy_data.c
    ${OPENSSL_ROOT}/crypto/x509/pcy_lib.c
    ${OPENSSL_ROOT}/crypto/x509/pcy_map.c
    ${OPENSSL_ROOT}/crypto/x509/pcy_node.c
    ${OPENSSL_ROOT}/crypto/x509/pcy_tree.c
    ${OPENSSL_ROOT}/crypto/x509/t_crl.c
    ${OPENSSL_ROOT}/crypto/x509/t_req.c
    ${OPENSSL_ROOT}/crypto/x509/t_x509.c
    ${OPENSSL_ROOT}/crypto/x509/v3_addr.c
    ${OPENSSL_ROOT}/crypto/x509/v3_admis.c
    ${OPENSSL_ROOT}/crypto/x509/v3_akeya.c
    ${OPENSSL_ROOT}/crypto/x509/v3_akid.c
    ${OPENSSL_ROOT}/crypto/x509/v3_asid.c
    ${OPENSSL_ROOT}/crypto/x509/v3_bcons.c
    ${OPENSSL_ROOT}/crypto/x509/v3_bitst.c
    ${OPENSSL_ROOT}/crypto/x509/v3_conf.c
    ${OPENSSL_ROOT}/crypto/x509/v3_cpols.c
    ${OPENSSL_ROOT}/crypto/x509/v3_crld.c
    ${OPENSSL_ROOT}/crypto/x509/v3_enum.c
    ${OPENSSL_ROOT}/crypto/x509/v3_extku.c
    ${OPENSSL_ROOT}/crypto/x509/v3_genn.c
    ${OPENSSL_ROOT}/crypto/x509/v3_ia5.c
    ${OPENSSL_ROOT}/crypto/x509/v3_info.c
    ${OPENSSL_ROOT}/crypto/x509/v3_int.c
    ${OPENSSL_ROOT}/crypto/x509/v3_ist.c
    ${OPENSSL_ROOT}/crypto/x509/v3_lib.c
    ${OPENSSL_ROOT}/crypto/x509/v3_ncons.c
    ${OPENSSL_ROOT}/crypto/x509/v3_pci.c
    ${OPENSSL_ROOT}/crypto/x509/v3_pcia.c
    ${OPENSSL_ROOT}/crypto/x509/v3_pcons.c
    ${OPENSSL_ROOT}/crypto/x509/v3_pku.c
    ${OPENSSL_ROOT}/crypto/x509/v3_pmaps.c
    ${OPENSSL_ROOT}/crypto/x509/v3_prn.c
    ${OPENSSL_ROOT}/crypto/x509/v3_purp.c
    ${OPENSSL_ROOT}/crypto/x509/v3_san.c
    ${OPENSSL_ROOT}/crypto/x509/v3_skid.c
    ${OPENSSL_ROOT}/crypto/x509/v3_sxnet.c
    ${OPENSSL_ROOT}/crypto/x509/v3_tlsf.c
    ${OPENSSL_ROOT}/crypto/x509/v3_utf8.c
    ${OPENSSL_ROOT}/crypto/x509/v3_utl.c
    ${OPENSSL_ROOT}/crypto/x509/v3err.c
    ${OPENSSL_ROOT}/crypto/x509/x509_att.c
    ${OPENSSL_ROOT}/crypto/x509/x509_cmp.c
    ${OPENSSL_ROOT}/crypto/x509/x509_d2.c
    ${OPENSSL_ROOT}/crypto/x509/x509_def.c
    ${OPENSSL_ROOT}/crypto/x509/x509_err.c
    ${OPENSSL_ROOT}/crypto/x509/x509_ext.c
    ${OPENSSL_ROOT}/crypto/x509/x509_lu.c
    ${OPENSSL_ROOT}/crypto/x509/x509_meth.c
    ${OPENSSL_ROOT}/crypto/x509/x509_obj.c
    ${OPENSSL_ROOT}/crypto/x509/x509_r2x.c
    ${OPENSSL_ROOT}/crypto/x509/x509_req.c
    ${OPENSSL_ROOT}/crypto/x509/x509_set.c
    ${OPENSSL_ROOT}/crypto/x509/x509_trust.c
    ${OPENSSL_ROOT}/crypto/x509/x509_txt.c
    ${OPENSSL_ROOT}/crypto/x509/x509_v3.c
    ${OPENSSL_ROOT}/crypto/x509/x509_vfy.c
    ${OPENSSL_ROOT}/crypto/x509/x509_vpm.c
    ${OPENSSL_ROOT}/crypto/x509/x509cset.c
    ${OPENSSL_ROOT}/crypto/x509/x509name.c
    ${OPENSSL_ROOT}/crypto/x509/x509rset.c
    ${OPENSSL_ROOT}/crypto/x509/x509spki.c
    ${OPENSSL_ROOT}/crypto/x509/x509type.c
    ${OPENSSL_ROOT}/crypto/x509/x_all.c
    ${OPENSSL_ROOT}/crypto/x509/x_attrib.c
    ${OPENSSL_ROOT}/crypto/x509/x_crl.c
    ${OPENSSL_ROOT}/crypto/x509/x_exten.c
    ${OPENSSL_ROOT}/crypto/x509/x_name.c
    ${OPENSSL_ROOT}/crypto/x509/x_pubkey.c
    ${OPENSSL_ROOT}/crypto/x509/x_req.c
    ${OPENSSL_ROOT}/crypto/x509/x_x509.c
    ${OPENSSL_ROOT}/crypto/x509/x_x509a.c
    ${OPENSSL_ROOT}/engines/e_capi.c
    ${OPENSSL_ROOT}/engines/e_padlock.c
    ${OPENSSL_ROOT}/providers/baseprov.c
    ${OPENSSL_ROOT}/providers/defltprov.c
    ${OPENSSL_ROOT}/providers/legacyprov.c
    ${OPENSSL_ROOT}/providers/nullprov.c
    ${OPENSSL_ROOT}/providers/prov_running.c
)

list(APPEND CRYPTO_SOURCES ${LIBCOMMON_COMMON_SOURCES})
list(APPEND CRYPTO_SOURCES ${LIBCOMMON_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES})
list(APPEND CRYPTO_SOURCES ${LIBDEFAULT_COMMON_SOURCES})
list(APPEND CRYPTO_SOURCES ${LIBDEFAULT_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES})
list(APPEND CRYPTO_SOURCES ${LIBCRYPTO_BUILD_ALL_GENERATED_SELECTED_PLATFORM_SOURCES})
list(APPEND CRYPTO_SOURCES ${LIBLEGACY_SOURCES})

if ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-armv4")
    list(APPEND CRYPTO_SOURCES
        ${OPENSSL_ROOT}/crypto/aes/aes_cbc.c
        ${OPENSSL_ROOT}/crypto/armcap.c
        ${OPENSSL_ROOT}/crypto/bn/bn_asm.c
        ${OPENSSL_ROOT}/crypto/camellia/camellia.c
        ${OPENSSL_ROOT}/crypto/camellia/cmll_cbc.c
        ${OPENSSL_ROOT}/crypto/rc4/rc4_enc.c
        ${OPENSSL_ROOT}/crypto/rc4/rc4_skey.c
        ${OPENSSL_ROOT}/crypto/whrlpool/wp_block.c
        ${OPENSSL_ROOT}/engines/e_afalg.c
    )
elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-aarch64")
    list(APPEND CRYPTO_SOURCES
        ${OPENSSL_ROOT}/crypto/aes/aes_cbc.c
        ${OPENSSL_ROOT}/crypto/aes/aes_core.c
        ${OPENSSL_ROOT}/crypto/armcap.c
        ${OPENSSL_ROOT}/crypto/bn/bn_asm.c
        ${OPENSSL_ROOT}/crypto/camellia/camellia.c
        ${OPENSSL_ROOT}/crypto/camellia/cmll_cbc.c
        ${OPENSSL_ROOT}/crypto/rc4/rc4_enc.c
        ${OPENSSL_ROOT}/crypto/rc4/rc4_skey.c
        ${OPENSSL_ROOT}/crypto/whrlpool/wp_block.c
        ${OPENSSL_ROOT}/engines/e_afalg.c
    )
elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "darwin64-x86_64-cc")
    list(APPEND CRYPTO_SOURCES
        ${OPENSSL_ROOT}/crypto/bn/asm/x86_64-gcc.c
        ${OPENSSL_ROOT}/crypto/bn/rsaz_exp.c
        ${OPENSSL_ROOT}/crypto/bn/rsaz_exp_x2.c
    )
elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "darwin64-arm64-cc")
    list(APPEND CRYPTO_SOURCES
        ${OPENSSL_ROOT}/crypto/aes/aes_cbc.c
        ${OPENSSL_ROOT}/crypto/aes/aes_core.c
        ${OPENSSL_ROOT}/crypto/armcap.c
        ${OPENSSL_ROOT}/crypto/bn/bn_asm.c
        ${OPENSSL_ROOT}/crypto/camellia/camellia.c
        ${OPENSSL_ROOT}/crypto/camellia/cmll_cbc.c
        ${OPENSSL_ROOT}/crypto/rc4/rc4_enc.c
        ${OPENSSL_ROOT}/crypto/rc4/rc4_skey.c
        ${OPENSSL_ROOT}/crypto/whrlpool/wp_block.c
    )
elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-x86_64")
    list(APPEND CRYPTO_SOURCES
        ${OPENSSL_ROOT}/crypto/bn/asm/x86_64-gcc.c
        ${OPENSSL_ROOT}/crypto/bn/rsaz_exp.c
        ${OPENSSL_ROOT}/crypto/bn/rsaz_exp_x2.c
        ${OPENSSL_ROOT}/engines/e_afalg.c
    )
# elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "linux-x86")
elseif ("${OPENSSL_SELECTED_PLATFORM}" STREQUAL "mingw64")
    list(APPEND CRYPTO_SOURCES
        ${OPENSSL_ROOT}/crypto/bn/asm/x86_64-gcc.c
        ${OPENSSL_ROOT}/crypto/bn/rsaz_exp.c
        ${OPENSSL_ROOT}/crypto/bn/rsaz_exp_x2.c
    )
endif()

panda_add_library(crypto_shared ${PANDA_DEFAULT_LIB_TYPE} ${CRYPTO_SOURCES})
panda_target_include_directories(crypto_shared SYSTEM
    PUBLIC ${CRYPTO_CONFIG_COMMON_PUBLIC_INCLUDES}
    PRIVATE ${CRYPTO_CONFIG_COMMON_PRIVATE_INCLUDES}
)
panda_target_compile_options(crypto_shared PRIVATE ${OPENSSL_CFLAGS_CC})
panda_target_compile_definitions(crypto_shared PRIVATE ${OPENSSL_DEFINITIONS})

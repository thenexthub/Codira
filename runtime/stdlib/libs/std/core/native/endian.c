/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <limits.h>
#include <stdint.h>

#if defined(_MSC_VER) || defined(__WIN32)

#include <stdlib.h>
#define bswap_16 _byteswap_ushort
#define bswap_32 _byteswap_ulong
#define bswap_64 _byteswap_uint64

#define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define BYTE_ORDER __BYTE_ORDER__

#elif defined(__APPLE__)

// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define bswap_16 OSSwapInt16
#define bswap_32 OSSwapInt32
#define bswap_64 OSSwapInt64

#define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define BYTE_ORDER __BYTE_ORDER__

#else

#include <byteswap.h>
#include <endian.h>

#endif

enum {
    O32_LITTLE_ENDIAN = 0x03020100UL,
    O32_BIG_ENDIAN = 0x00010203UL,
    O32_PDP_ENDIAN = 0x01000302UL,      /* DEC PDP-11 (aka ENDIAN_LITTLE_WORD) */
    O32_HONEYWELL_ENDIAN = 0x02030001UL /* Honeywell 316 (aka ENDIAN_BIG_WORD) */
};

static const union {
    unsigned char bytes[4];
    uint32_t value;
} CONST_O32_HOST_ORDER = {.bytes = {0, 1, 2, 3}};

#define O32_HOST_ORDER (CONST_O32_HOST_ORDER.value)

enum {
    // Big-Endian, for example, Solaris
    PLATFORM_BIG_ENDIAN = 1,
    // Little-Endian, for example, Linux x86-64
    PLATFORM_LITTLE_ENDIAN = 2,
    // Middle-Endian, for example, PDP-11 Minicomputer
    PLATFORM_PDP_ENDIAN = 3,
    // Unknown endianness
    PLATFORM_UNKNOWN_BYTEORDER = 4
};

/**
 * @brief Returns the endianness of the running platform.
 * All known endiannesses are detected here, but only BigEndian and LittleEndian are supported.
 *
 * @return uint8_t
 */
extern uint8_t CODE_PlatformEndian(void)
{
    if (O32_HOST_ORDER == O32_BIG_ENDIAN) {
        return PLATFORM_BIG_ENDIAN;
    } else if (O32_HOST_ORDER == PLATFORM_LITTLE_ENDIAN) {
        return PLATFORM_LITTLE_ENDIAN;
    } else if (O32_HOST_ORDER == PLATFORM_PDP_ENDIAN) {
        return PLATFORM_PDP_ENDIAN;
    } else if (O32_HOST_ORDER == O32_HONEYWELL_ENDIAN) {
        return PLATFORM_UNKNOWN_BYTEORDER;
    } else {
        return PLATFORM_LITTLE_ENDIAN;
    }
}

extern uint64_t CODE_Bswap64(uint64_t n)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    return bswap_64(n);
#else
    return n;
#endif
}

extern uint32_t CODE_Bswap32(uint32_t n)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    return bswap_32(n);
#else
    return n;
#endif
}

extern uint16_t CODE_Bswap16(uint16_t n)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    return bswap_16(n);
#else
    return n;
#endif
}

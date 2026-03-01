/*-
 * Copyright (c) 1997 Niklas Hallqvist.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

/**
 * @file sys/endian.h
 * @brief Endianness utilities.
 */

#include <sys/cdefs.h>

#include <stdint.h>

__BEGIN_DECLS

#define __swap16 __builtin_bswap16
#define __swap32 __builtin_bswap32
#define __swap64(x) __BIONIC_CAST(static_cast,uint64_t,__builtin_bswap64(x))

/* POSIX. */

/** The value of BYTE_ORDER on little-endian systems. */
#define LITTLE_ENDIAN 1234

/** The value of BYTE_ORDER on big-endian systems. */
#define BIG_ENDIAN 4321

/** Android is always little-endian. */
#define BYTE_ORDER LITTLE_ENDIAN

/** Swap big-endian 16-bit quantity to host (little-endian) byte order. */
#define be16toh(x) __swap16(x)
/** Swap big-endian 32-bit quantity to host (little-endian) byte order. */
#define be32toh(x) __swap32(x)
/** Swap big-endian 64-bit quantity to host (little-endian) byte order. */
#define be64toh(x) __swap64(x)

/** Swap host (little-endian) 16-bit quantity to big-endian. */
#define htobe16(x) __swap16(x)
/** Swap host (little-endian) 32-bit quantity to big-endian. */
#define htobe32(x) __swap32(x)
/** swap host (little-endian) 64-bit quantity to big-endian. */
#define htobe64(x) __swap64(x)

/** No-op conversion of host (little-endian) 16-bit quantity to little-endian. */
#define htole16(x) (x)
/** No-op conversion of host (little-endian) 32-bit quantity to little-endian. */
#define htole32(x) (x)
/** No-op conversion of host (little-endian) 64-bit quantity to little-endian. */
#define htole64(x) (x)

/** No-op conversion of little-endian 16-bit quantity to host (little-endian) byte order. */
#define le16toh(x) (x)
/** No-op conversion of little-endian 32-bit quantity to host (little-endian) byte order. */
#define le32toh(x) (x)
/** No-op conversion of little-endian 64-bit quantity to host (little-endian) byte order. */
#define le64toh(x) (x)

/** Synonym for BIG_ENDIAN. */
#define _BIG_ENDIAN	BIG_ENDIAN
/** Synonym for BYTE_ORDER. */
#define _BYTE_ORDER BYTE_ORDER
/** Synonym for LITTLE_ENDIAN. */
#define _LITTLE_ENDIAN LITTLE_ENDIAN

/** Synonym for BIG_ENDIAN. */
#define __BIG_ENDIAN BIG_ENDIAN
/** Synonym for BYTE_ORDER. */
#define __BYTE_ORDER BYTE_ORDER
/** Synonym for LITTLE_ENDIAN. */
#define __LITTLE_ENDIAN LITTLE_ENDIAN

/** The byte order of bitfields. Accidental Linux header leakage. */
#define __LITTLE_ENDIAN_BITFIELD


/*
 * POSIX has these in <arpa/inet.h>,
 * but we have them here for glibc source compatibility.
 */

/** Swap host (little-endian) 32-bit quantity to network (big-endian). */
uint32_t htonl(uint32_t __x) __attribute_const__;
#define htonl(x) __swap32(x)

/** Swap host (little-endian) 16-bit quantity to network (big-endian). */
uint16_t htons(uint16_t __x) __attribute_const__;
#define htons(x) __swap16(x)

/** Swap network (big-endian) 32-bit quantity to host (little-endian). */
uint32_t ntohl(uint32_t __x) __attribute_const__;
#define ntohl(x) __swap32(x)

/** Swap network (big-endian) 16-bit quantity to host (little-endian). */
uint16_t ntohs(uint16_t __x) __attribute_const__;
#define ntohs(x) __swap16(x)


/* Bionic additions */

/** Swap host (little-endian) 64-bit quantity to network (big-endian). */
#define htonq(x) __swap64(x)

/** Swap network (big-endian) 64-bit quantity to host (little-endian). */
#define ntohq(x) __swap64(x)


/* BSD extensions unconditionally exposed by bionic. */

/** The value of BYTE_ORDER on PDP-endian systems. */
#define PDP_ENDIAN 3412
/** Synonym for PDP_ENDIAN. */
#define _PDP_ENDIAN	PDP_ENDIAN

/** In-place byte swap of 32-bit argument. */
#define	NTOHL(x) (x) = ntohl(__BIONIC_CAST(static_cast,u_int32_t,(x)))
/** In-place byte swap of 16-bit argument. */
#define	NTOHS(x) (x) = ntohs(__BIONIC_CAST(static_cast,u_int16_t,(x)))
/** In-place byte swap of 32-bit argument. */
#define	HTONL(x) (x) = htonl(__BIONIC_CAST(static_cast,u_int32_t,(x)))
/** In-place byte swap of 16-bit argument. */
#define	HTONS(x) (x) = htons(__BIONIC_CAST(static_cast,u_int16_t,(x)))


/* glibc extensions. */

/** Swap big-endian 16-bit quantity to host (little-endian). */
#define betoh16(x) __swap16(x)
/** Swap big-endian 32-bit quantity to host (little-endian). */
#define betoh32(x) __swap32(x)
/** Swap big-endian 64-bit quantity to host (little-endian). */
#define betoh64(x) __swap64(x)

/** No-op conversion of little-endian 16-bit quantity to host (little-endian). */
#define letoh16(x) (x)
/** No-op conversion of little-endian 32-bit quantity to host (little-endian). */
#define letoh32(x) (x)
/** No-op conversion of little-endian 64-bit quantity to host (little-endian). */
#define letoh64(x) (x)

__END_DECLS

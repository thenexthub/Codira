// Copyright (C) 2025 The Android Open Source Project
// SPDX-License-Identifier: BSD-2-Clause

#include <endian.h>

#include "header_checks.h"

static void endian_h() {
#if !defined(__GLIBC__) // glibc still has this POSIX 2024 header as extensions.
  MACRO(BYTE_ORDER);
  MACRO(LITTLE_ENDIAN);
  MACRO(BIG_ENDIAN);

  // TODO: better support for function-like macros.
  be16toh(1234);
  be32toh(1234);
  be64toh(1234);

  htobe16(1234);
  htobe32(1234);
  htobe64(1234);

  htole16(1234);
  htole32(1234);
  htole64(1234);

  le16toh(1234);
  le32toh(1234);
  le64toh(1234);

  TYPE(uint16_t);
  TYPE(uint32_t);
  TYPE(uint64_t);
#endif
}

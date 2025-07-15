//===--- UUID.cpp - UUID generation ---------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This is an interface over the standard OSF uuid library that gives UUIDs
// sound value semantics and operators.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/Basic/UUID.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallString.h"

// WIN32 doesn't natively support <uuid/uuid.h>. Instead, we use Win32 APIs.
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <objbase.h>
#include <string>
#include <algorithm>
#else
#include <uuid/uuid.h>
#endif

using namespace language;

language::UUID::UUID(FromRandom_t) {
#if defined(_WIN32)
  ::UUID uuid;
  ::CoCreateGuid(&uuid);

  memcpy(Value, &uuid, Size);
#else
  uuid_generate_random(Value);
#endif
}

language::UUID::UUID(FromTime_t) {
#if defined(_WIN32)
  ::UUID uuid;
  ::CoCreateGuid(&uuid);

  memcpy(Value, &uuid, Size);
#else
  uuid_generate_time(Value);
#endif
}

language::UUID::UUID() {
#if defined(_WIN32)
  ::UUID uuid = *((::UUID *)&Value);
  UuidCreateNil(&uuid);

  memcpy(Value, &uuid, Size);
#else
  uuid_clear(Value);
#endif
}

std::optional<language::UUID> language::UUID::fromString(const char *s) {
#if defined(_WIN32)
  RPC_CSTR t = const_cast<RPC_CSTR>(reinterpret_cast<const unsigned char*>(s));

  ::UUID uuid;
  RPC_STATUS status = UuidFromStringA(t, &uuid);
  if (status == RPC_S_INVALID_STRING_UUID) {
    return std::nullopt;
  }

  language::UUID result = UUID();
  memcpy(result.Value, &uuid, Size);
  return result;
#else
  language::UUID result;
  if (uuid_parse(s, result.Value))
    return std::nullopt;
  return result;
#endif
}

void language::UUID::toString(toolchain::SmallVectorImpl<char> &out) const {
  out.resize(UUID::StringBufferSize);
#if defined(_WIN32)
  ::UUID uuid;
  memcpy(&uuid, Value, Size);

  RPC_CSTR str;
  UuidToStringA(&uuid, &str);

  char* signedStr = reinterpret_cast<char*>(str);
  memcpy(out.data(), signedStr, StringBufferSize);
  toolchain::transform(out, std::begin(out), toupper);
#else
  uuid_unparse_upper(Value, out.data());
#endif
  // Pop off the null terminator.
  assert(out.back() == '\0' && "did not null-terminate?!");
  out.pop_back();
}

int language::UUID::compare(UUID y) const {
#if defined(_WIN32)
  RPC_STATUS s;
  ::UUID uuid1;
  memcpy(&uuid1, Value, Size);

  ::UUID uuid2;
  memcpy(&uuid2, y.Value, Size);

  return UuidCompare(&uuid1, &uuid2, &s);
#else
  return uuid_compare(Value, y.Value);
#endif
}

toolchain::raw_ostream &language::operator<<(toolchain::raw_ostream &os, UUID uuid) {
  toolchain::SmallString<UUID::StringBufferSize> buf;
  uuid.toString(buf);
  os << buf;
  return os;
}

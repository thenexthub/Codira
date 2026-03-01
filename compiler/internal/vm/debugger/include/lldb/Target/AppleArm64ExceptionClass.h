//===-- AppleArm64ExceptionClass.h ------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TARGET_APPLEARM64EXCEPTIONCLASS_H
#define LLDB_TARGET_APPLEARM64EXCEPTIONCLASS_H

#include <cstdint>

namespace lldb_private {

enum class AppleArm64ExceptionClass : unsigned {
#define APPLE_ARM64_EXCEPTION_CLASS(Name, Code) Name = Code,
#include "AppleArm64ExceptionClass.def"
};

/// Get the Apple ARM64 exception class encoded within \p esr.
inline AppleArm64ExceptionClass getAppleArm64ExceptionClass(uint32_t esr) {
  /*
   * Exception Syndrome Register
   *
   *  31  26 25 24               0
   * +------+--+------------------+
   * |  EC  |IL|       ISS        |
   * +------+--+------------------+
   *
   * EC  - Exception Class
   * IL  - Instruction Length
   * ISS - Instruction Specific Syndrome
   */
  return static_cast<AppleArm64ExceptionClass>(esr >> 26);
}

inline const char *toString(AppleArm64ExceptionClass EC) {
  switch (EC) {
#define APPLE_ARM64_EXCEPTION_CLASS(Name, Code)                                \
  case AppleArm64ExceptionClass::Name:                                         \
    return #Name;
#include "AppleArm64ExceptionClass.def"
  }
  return "Unknown Exception Class";
}

} // namespace lldb_private

#endif // LLDB_TARGET_APPLEARM64EXCEPTIONCLASS_H

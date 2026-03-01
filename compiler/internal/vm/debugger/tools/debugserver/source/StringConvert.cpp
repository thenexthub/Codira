//===-- StringConvert.cpp -------------------------------------------------===//
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

#include <cstdlib>

#include "StringConvert.h"

namespace StringConvert {

int64_t ToSInt64(const char *s, int64_t fail_value, int base,
                 bool *success_ptr) {
  if (s && s[0]) {
    char *end = nullptr;
    int64_t uval = ::strtoll(s, &end, base);
    if (*end == '\0') {
      if (success_ptr)
        *success_ptr = true;
      return uval; // All characters were used, return the result
    }
  }
  if (success_ptr)
    *success_ptr = false;
  return fail_value;
}

uint64_t ToUInt64(const char *s, uint64_t fail_value, int base,
                  bool *success_ptr) {
  if (s && s[0]) {
    char *end = nullptr;
    uint64_t uval = ::strtoull(s, &end, base);
    if (*end == '\0') {
      if (success_ptr)
        *success_ptr = true;
      return uval; // All characters were used, return the result
    }
  }
  if (success_ptr)
    *success_ptr = false;
  return fail_value;
}

double ToDouble(const char *s, double fail_value, bool *success_ptr) {
  if (s && s[0]) {
    char *end = nullptr;
    double val = strtod(s, &end);
    if (*end == '\0') {
      if (success_ptr)
        *success_ptr = true;
      return val; // All characters were used, return the result
    }
  }
  if (success_ptr)
    *success_ptr = false;
  return fail_value;
}

} // namespace StringConvert

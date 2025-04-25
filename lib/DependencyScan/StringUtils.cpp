//===- StringUtils.cpp - Routines for manipulating swiftscan_string_ref_t -===//
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

#include "language/DependencyScan/StringUtils.h"
#include <string>
#include <vector>

namespace language {
namespace c_string_utils {

swiftscan_string_ref_t create_null() {
  swiftscan_string_ref_t str;
  str.data = nullptr;
  str.length = 0;
  return str;
}

swiftscan_string_ref_t create_clone(const char *string) {
  if (!string)
    return create_null();

  if (string[0] == '\0')
    return create_null();

  swiftscan_string_ref_t str;
  str.data = strdup(string);
  str.length = strlen(string);
  return str;
}

swiftscan_string_set_t *create_set(const std::vector<std::string> &strings) {
  swiftscan_string_set_t *set = new swiftscan_string_set_t;
  set->count = strings.size();
  set->strings = new swiftscan_string_ref_t[set->count];
  for (unsigned SI = 0, SE = set->count; SI < SE; ++SI)
    set->strings[SI] = create_clone(strings[SI].c_str());
  return set;
}

swiftscan_string_set_t *create_set(int count, const char **strings) {
  swiftscan_string_set_t *set = new swiftscan_string_set_t;
  set->count = count;
  set->strings = new swiftscan_string_ref_t[set->count];
  for (unsigned SI = 0, SE = set->count; SI < SE; ++SI)
    set->strings[SI] = create_clone(strings[SI]);
  return set;
}

swiftscan_string_set_t *create_empty_set() {
  swiftscan_string_set_t *set = new swiftscan_string_set_t;
  set->count = 0;
  return set;
}

const char *get_C_string(swiftscan_string_ref_t string) {
  return static_cast<const char *>(string.data);
}
} // namespace c_string_utils
} // namespace language

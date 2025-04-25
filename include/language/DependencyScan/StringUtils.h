//===----- StringUtils.h - Managed C String Utility Functions -----*- C -*-===//
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

#include "language-c/CommonString/CommonString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"
#include <string>
#include <vector>

//=== Private Utility Functions--------------------------------------------===//
namespace language {
namespace c_string_utils {

/// Create null string
swiftscan_string_ref_t create_null();

/// Create a \c swiftscan_string_ref_t object from a nul-terminated C string.  New
/// \c swiftscan_string_ref_t will contain a copy of \p string.
swiftscan_string_ref_t create_clone(const char *string);

/// Create an array of \c swiftscan_string_ref_t objects from a vector of C++ strings using the
/// create_clone routine.
swiftscan_string_set_t *create_set(const std::vector<std::string> &strings);

/// Create an array of swiftscan_string_ref_t objects from an array of C strings using the
/// create_clone routine.
swiftscan_string_set_t *create_set(int count, const char **strings);

/// Create an empty array of swiftscan_string_ref_t objects
swiftscan_string_set_t *create_empty_set();

/// Retrieve the character data associated with the given string.
const char *get_C_string(swiftscan_string_ref_t string);
}
}

//===--- CommonString.h - C API for Codira Dependency Scanning ---*- C -*-===//
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

 #ifndef LANGUAGE_C_LIB_LANGUAGE_COMMON_STRING_H
 #define LANGUAGE_C_LIB_LANGUAGE_COMMON_STRING_H

 #include <stdbool.h>
 #include <stddef.h>
 #include <stdint.h>

 /**
  * A character string used to pass around dependency scan result metadata.
  * Lifetime of the string is strictly tied to the object whose field it
  * represents. When the owning object is released, string memory is freed.
  */
 typedef struct {
   const void *data;
   size_t length;
 } languagescan_string_ref_t;

 typedef struct {
   languagescan_string_ref_t *strings;
   size_t count;
 } languagescan_string_set_t;

 #endif // LANGUAGE_C_LIB_LANGUAGE_COMMON_STRING_H

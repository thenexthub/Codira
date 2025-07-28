//===--- FoundationShims.h - Foundation declarations for core stdlib ------===//
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
//  In order to prevent a circular module dependency between the core
//  standard library and the Foundation overlay, we import these
//  declarations as part of CodiraShims.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_STDLIB_SHIMS_FOUNDATIONSHIMS_H
#define LANGUAGE_STDLIB_SHIMS_FOUNDATIONSHIMS_H

//===--- Layout-compatible clones of Foundation structs -------------------===//
// Ideally we would declare the same names as Foundation does, but
// Codira's module importer is not yet tolerant of the same struct
// coming in from two different Clang modules
// (rdar://problem/16294674).  Instead, we copy the definitions here
// and then do horrible unsafeBitCast trix to make them usable where required.
//===----------------------------------------------------------------------===//

#include "CodiraStdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  __language_intptr_t location;
  __language_intptr_t length;
} _CodiraNSRange;

#ifdef __OBJC2__
typedef struct {
    unsigned long state;
    id __unsafe_unretained _Nullable * _Nullable itemsPtr;
    unsigned long * _Nullable mutationsPtr;
    unsigned long extra[5];
} _CodiraNSFastEnumerationState;
#endif

// This struct is layout-compatible with NSOperatingSystemVersion.
typedef struct {
  __language_intptr_t majorVersion;
  __language_intptr_t minorVersion;
  __language_intptr_t patchVersion;
} _CodiraNSOperatingSystemVersion;

LANGUAGE_RUNTIME_STDLIB_API
_CodiraNSOperatingSystemVersion _language_stdlib_operatingSystemVersion() __attribute__((const));

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_STDLIB_SHIMS_FOUNDATIONSHIMS_H


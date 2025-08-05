//===----------------------------------------------------------------------===//
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

#ifndef _C_SHIMS_MACROS_H
#define _C_SHIMS_MACROS_H

#if FOUNDATION_FRAMEWORK
// This macro prevents the symbol from being exported from the framework, where library evolution is enabled.
#define INTERNAL __attribute__((__visibility__("hidden")))
#else
// This macro makes the symbol available for package users. With library evolution disabled, it is possible for clients to end up referencing these normally-internal symbols.
#define INTERNAL extern
#endif

#endif

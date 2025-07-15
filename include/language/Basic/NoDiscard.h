//===--- NoDiscard.h ------------------------------------------------------===//
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

#ifndef LANGUAGE_BASIC_NODISCARD_H
#define LANGUAGE_BASIC_NODISCARD_H

#if __cplusplus > 201402l && __has_cpp_attribute(nodiscard)
#define LANGUAGE_NODISCARD [[nodiscard]]
#elif __has_cpp_attribute(clang::warn_unused_result)
#define LANGUAGE_NODISCARD [[clang::warn_unused_result]]
#else
#define LANGUAGE_NODISCARD
#endif

#endif

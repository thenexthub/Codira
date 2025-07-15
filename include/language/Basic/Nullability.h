//===--- Nullability.h ----------------------------------------------------===//
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

#ifndef LANGUAGE_BASIC_NULLABILITY_H
#define LANGUAGE_BASIC_NULLABILITY_H

// TODO: These macro definitions are duplicated in Visibility.h. Move
// them to a single file if we find a location that both Visibility.h and
// BridgedCodiraObject.h can import.
#if __has_feature(nullability)
// Provide macros to temporarily suppress warning about the use of
// _Nullable and _Nonnull.
#define LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS                                    \
  _Pragma("clang diagnostic push")                                             \
      _Pragma("clang diagnostic ignored \"-Wnullability-extension\"")
#define LANGUAGE_END_NULLABILITY_ANNOTATIONS _Pragma("clang diagnostic pop")

#define LANGUAGE_BEGIN_ASSUME_NONNULL _Pragma("clang assume_nonnull begin")
#define LANGUAGE_END_ASSUME_NONNULL _Pragma("clang assume_nonnull end")
#else
// #define _Nullable and _Nonnull to nothing if we're not being built
// with a compiler that supports them.
#define _Nullable
#define _Nonnull
#define _Null_unspecified
#define LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS
#define LANGUAGE_END_NULLABILITY_ANNOTATIONS
#define LANGUAGE_BEGIN_ASSUME_NONNULL
#define LANGUAGE_END_ASSUME_NONNULL
#endif

#endif

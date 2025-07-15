//===--- NamespaceMacros.h - Macros for inline namespaces -------*- C++ -*-===//
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
// Macros that conditionally define an inline namespace so that symbols used in
// multiple places (such as in the compiler and in the runtime library) can be
// given distinct mangled names in different contexts without affecting client
// usage in source.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_DEMANGLING_NAMESPACE_MACROS_H
#define LANGUAGE_DEMANGLING_NAMESPACE_MACROS_H

#if defined(__cplusplus)

#if defined(LANGUAGE_INLINE_NAMESPACE)
#define LANGUAGE_BEGIN_INLINE_NAMESPACE inline namespace LANGUAGE_INLINE_NAMESPACE {
#define LANGUAGE_END_INLINE_NAMESPACE }
#else
#define LANGUAGE_BEGIN_INLINE_NAMESPACE
#define LANGUAGE_END_INLINE_NAMESPACE
#endif

#endif

#endif // LANGUAGE_DEMANGLING_NAMESPACE_MACROS_H

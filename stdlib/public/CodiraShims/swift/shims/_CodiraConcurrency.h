//===--- _CodiraConcurrency.h - Codira Concurrency Support --------*- C++ -*-===//
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
//  Defines types and support functions for the Codira concurrency model.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CONCURRENCY_H
#define LANGUAGE_CONCURRENCY_H

#ifdef __cplusplus
namespace language {
extern "C" {
#endif

typedef struct _CodiraContext {
  struct _CodiraContext *parentContext;
} _CodiraContext;

#ifdef __cplusplus
} // extern "C"
} // namespace language
#endif

#endif // LANGUAGE_CONCURRENCY_H

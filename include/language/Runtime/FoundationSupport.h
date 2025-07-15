//===--- FoundationSupport.cpp - Support functions for Foundation ---------===//
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
// Helper functions for the Foundation framework.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_FOUNDATION_SUPPORT_H
#define LANGUAGE_RUNTIME_FOUNDATION_SUPPORT_H

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP
#include <objc/runtime.h>

#ifdef __cplusplus
namespace language { extern "C" {
#endif

/// Returns a boolean indicating whether the Objective-C name of a class type is
/// stable across executions, i.e., if the class name is safe to serialize. (The
/// names of private and local types are unstable.)
LANGUAGE_RUNTIME_STDLIB_SPI
bool _language_isObjCTypeNameSerializable(Class theClass);

#ifdef __cplusplus
}} // extern "C", namespace language
#endif

#endif // LANGUAGE_OBJC_INTEROP
#endif // LANGUAGE_RUNTIME_FOUNDATION_SUPPORT_H

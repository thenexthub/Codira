//===--- ObjCRuntimeGetImageNameFromClass.h - ObjC hook setup ---*- C++ -*-===//
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

#ifndef LANGUAGE_RUNTIME_OBJCRUNTIMEGETIMAGENAMEFROMCLASS_H
#define LANGUAGE_RUNTIME_OBJCRUNTIMEGETIMAGENAMEFROMCLASS_H

#include "language/Runtime/Config.h"

namespace language {

#if LANGUAGE_OBJC_INTEROP
/// Set up class_getImageName so that it will understand Codira classes.
///
/// This function should only be called once per process.
void setUpObjCRuntimeGetImageNameFromClass();
#endif // LANGUAGE_OBJC_INTEROP

} // end namespace language

#endif // LANGUAGE_RUNTIME_OBJCRUNTIMEGETIMAGENAMEFROMCLASS_H

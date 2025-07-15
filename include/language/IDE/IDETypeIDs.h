//===--- IDETypeIDs.h - IDE Type Ids ----------------------------*- C++ -*-===//
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
//  This file defines TypeID support for IDE types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IDE_IDETYPEIDS_H
#define LANGUAGE_IDE_IDETYPEIDS_H

#include "language/Basic/TypeID.h"
namespace language {

#define LANGUAGE_TYPEID_ZONE IDETypes
#define LANGUAGE_TYPEID_HEADER "language/IDE/IDETypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"

} // end namespace language

#endif // LANGUAGE_IDE_IDETYPEIDS_H

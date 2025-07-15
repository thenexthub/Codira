//===--- ClassLayout.cpp - Layout of class instances ---------------------===//
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
//  This file implements algorithms for laying out class instances.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"

#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "ClassLayout.h"
#include "TypeInfo.h"

using namespace language;
using namespace irgen;

ClassLayout::ClassLayout(const StructLayoutBuilder &builder,
                         ClassMetadataOptions options,
                         toolchain::Type *classTy,
                         ArrayRef<Field> allStoredProps,
                         ArrayRef<FieldAccess> allFieldAccesses,
                         ArrayRef<ElementLayout> allElements,
                         Size headerSize)
  : MinimumAlign(builder.getAlignment()),
    MinimumSize(builder.getSize()),
    IsFixedLayout(builder.isFixedLayout()),
    Options(options),
    Ty(classTy),
    HeaderSize(headerSize),
    AllStoredProperties(allStoredProps),
    AllFieldAccesses(allFieldAccesses),
    AllElements(allElements) { }

Size ClassLayout::getInstanceStart() const {
  ArrayRef<ElementLayout> elements = AllElements;
  while (!elements.empty()) {
    auto element = elements.front();
    elements = elements.drop_front();

    // Ignore empty elements.
    if (element.isEmpty()) {
      continue;
    } else if (element.hasByteOffset()) {
      // FIXME: assumes layout is always sequential!
      return element.getByteOffset();
    } else {
      // We used to crash for classes that have an empty and a resilient field
      // during initialization.
      //   class CrashInInit {
      //     var empty = EmptyStruct()
      //     var resilient = ResilientThing()
      //   }
      // What happened was that for such a class we we would compute a
      // instanceStart of 0. The shared cache builder would then slide the value
      // of the constant ivar offset for the empty field from 0 to 16. However
      // the field offset for empty fields is assume to be zero and the runtime
      // does not compute a different value for the empty field and so the field
      // offset for the empty field stays 0. The runtime then trys to reconcile
      // the field offset and the ivar offset trying to write to the ivar
      // offset. However, the ivar offset is marked as constant and so we
      // crashed.
      // This can be avoided by correctly computing the instanceStart for such a
      // class to be 16 such that the shared cache builder does not update the
      // value of the empty field.
      if (!Options.contains(ClassMetadataFlags::ClassHasObjCAncestry))
        return HeaderSize;
      return Size(0);
    }
  }

  // If there are no non-empty elements, just return the computed size.
  return getSize();
}

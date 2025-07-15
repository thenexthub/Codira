//===--- DiverseStack.cpp - Out-of-line code for the heterogeneous stack --===//
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
//  This file implements the small amount of code for the heterogeneous
//  stack and list classes.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/Basic/DiverseList.h"
#include "language/Basic/DiverseStack.h"
using namespace language;

void DiverseStackBase::pushNewStorageSlow(std::size_t needed) {
  bool wasInline = isAllocatedInline();

  std::size_t capacity = End - Allocated;
  std::size_t requiredCapacity = capacity + needed;
  do {
    capacity = 2 * capacity + 16;
  } while (capacity < requiredCapacity);

  assert(capacity % 16 == 0 && "not allocating multiple of alignment");

  char *oldAllocation = Allocated;
  char *oldBegin = Begin;
  std::size_t oldSize = (std::size_t) (End - oldBegin);

  Allocated = new char[capacity];
  End = Allocated + capacity;
  Begin = End - oldSize;
  std::memcpy(Begin, oldBegin, oldSize);

  Begin -= needed;

  if (!wasInline) delete[] oldAllocation;
}  

char *DiverseListBase::addNewStorageSlow(std::size_t needed) {
  bool wasInline = isAllocatedInline();

  std::size_t capacity = EndOfAllocation - Begin;
  std::size_t requiredCapacity = capacity + needed;
  do {
    capacity = 2 * capacity + 16;
  } while (capacity < requiredCapacity);

  assert(capacity % 16 == 0 && "not allocating multiple of alignment");

  char *oldBegin = Begin;
  char *oldEnd = End;
  std::size_t oldSize = (std::size_t) (oldEnd - oldBegin);

  Begin = new char[capacity];
  EndOfAllocation = Begin + capacity;
  End = Begin + oldSize + needed;
  std::memcpy(Begin, oldBegin, oldSize);

  if (!wasInline) delete[] oldBegin;

  return Begin + oldSize;
}  

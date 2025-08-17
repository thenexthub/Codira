//===--- InterpStack.cpp - Stack implementation for the VM ------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "InterpStack.h"
#include "Boolean.h"
#include "FixedPoint.h"
#include "Floating.h"
#include "Integral.h"
#include "MemberPointer.h"
#include "Pointer.h"
#include <cassert>
#include <cstdlib>

using namespace language::Core;
using namespace language::Core::interp;

InterpStack::~InterpStack() {
  if (Chunk && Chunk->Next)
    std::free(Chunk->Next);
  if (Chunk)
    std::free(Chunk);
  Chunk = nullptr;
  StackSize = 0;
#ifndef NDEBUG
  ItemTypes.clear();
#endif
}

// We keep the last chunk around to reuse.
void InterpStack::clear() {
  if (!Chunk)
    return;

  if (Chunk->Next)
    std::free(Chunk->Next);

  assert(Chunk);
  StackSize = 0;
#ifndef NDEBUG
  ItemTypes.clear();
#endif
}

void InterpStack::clearTo(size_t NewSize) {
  assert(NewSize <= size());
  size_t ToShrink = size() - NewSize;
  if (ToShrink == 0)
    return;

  shrink(ToShrink);
  assert(size() == NewSize);
}

void *InterpStack::grow(size_t Size) {
  assert(Size < ChunkSize - sizeof(StackChunk) && "Object too large");

  if (!Chunk || sizeof(StackChunk) + Chunk->size() + Size > ChunkSize) {
    if (Chunk && Chunk->Next) {
      Chunk = Chunk->Next;
    } else {
      StackChunk *Next = new (std::malloc(ChunkSize)) StackChunk(Chunk);
      if (Chunk)
        Chunk->Next = Next;
      Chunk = Next;
    }
  }

  auto *Object = reinterpret_cast<void *>(Chunk->End);
  Chunk->End += Size;
  StackSize += Size;
  return Object;
}

void *InterpStack::peekData(size_t Size) const {
  assert(Chunk && "Stack is empty!");

  StackChunk *Ptr = Chunk;
  while (Size > Ptr->size()) {
    Size -= Ptr->size();
    Ptr = Ptr->Prev;
    assert(Ptr && "Offset too large");
  }

  return reinterpret_cast<void *>(Ptr->End - Size);
}

void InterpStack::shrink(size_t Size) {
  assert(Chunk && "Chunk is empty!");

  while (Size > Chunk->size()) {
    Size -= Chunk->size();
    if (Chunk->Next) {
      std::free(Chunk->Next);
      Chunk->Next = nullptr;
    }
    Chunk->End = Chunk->start();
    Chunk = Chunk->Prev;
    assert(Chunk && "Offset too large");
  }

  Chunk->End -= Size;
  StackSize -= Size;

#ifndef NDEBUG
  size_t TypesSize = 0;
  for (PrimType T : ItemTypes)
    TYPE_SWITCH(T, { TypesSize += aligned_size<T>(); });

  size_t StackSize = size();
  while (TypesSize > StackSize) {
    TYPE_SWITCH(ItemTypes.back(), {
      TypesSize -= aligned_size<T>();
      ItemTypes.pop_back();
    });
  }
  assert(TypesSize == StackSize);
#endif
}

void InterpStack::dump() const {
#ifndef NDEBUG
  toolchain::errs() << "Items: " << ItemTypes.size() << ". Size: " << size() << '\n';
  if (ItemTypes.empty())
    return;

  size_t Index = 0;
  size_t Offset = 0;

  // The type of the item on the top of the stack is inserted to the back
  // of the vector, so the iteration has to happen backwards.
  for (auto TyIt = ItemTypes.rbegin(); TyIt != ItemTypes.rend(); ++TyIt) {
    Offset += align(primSize(*TyIt));

    toolchain::errs() << Index << '/' << Offset << ": ";
    TYPE_SWITCH(*TyIt, {
      const T &V = peek<T>(Offset);
      toolchain::errs() << V;
    });
    toolchain::errs() << '\n';

    ++Index;
  }
#endif
}

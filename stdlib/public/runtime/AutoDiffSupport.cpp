//===--- AutoDiffSupport.cpp ----------------------------------*- C++ -*---===//
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

#include "AutoDiffSupport.h"
#include "language/ABI/Metadata.h"
#include "language/Runtime/HeapObject.h"
#include "toolchain/ADT/SmallVector.h"
#include <new>

using namespace language;
using namespace toolchain;

LANGUAGE_CC(language)
static void destroyLinearMapContext(LANGUAGE_CONTEXT HeapObject *obj) {
  static_cast<AutoDiffLinearMapContext *>(obj)->~AutoDiffLinearMapContext();
  free(obj);
}

/// Heap metadata for a linear map context.
static FullMetadata<HeapMetadata> linearMapContextHeapMetadata = {
  {
    {
      /*type layout*/ nullptr,
    },
    {
      &destroyLinearMapContext
    },
    {
      /*value witness table*/ nullptr
    }
  },
  {
    MetadataKind::Opaque
  }
};

AutoDiffLinearMapContext::AutoDiffLinearMapContext()
    : HeapObject(&linearMapContextHeapMetadata) {
}

AutoDiffLinearMapContext::AutoDiffLinearMapContext(
    const Metadata *topLevelLinearMapContextMetadata)
    : HeapObject(&linearMapContextHeapMetadata) {
  allocatedContextObjects.push_back(AllocatedContextObjectRecord{
      topLevelLinearMapContextMetadata, projectTopLevelSubcontext()});
}

void *AutoDiffLinearMapContext::projectTopLevelSubcontext() const {
  auto offset = alignTo(
      sizeof(AutoDiffLinearMapContext), alignof(AutoDiffLinearMapContext));
  return const_cast<uint8_t *>(
      reinterpret_cast<const uint8_t *>(this) + offset);
}

void *AutoDiffLinearMapContext::allocate(size_t size) {
  return allocator.Allocate(size, alignof(AutoDiffLinearMapContext));
}

void *AutoDiffLinearMapContext::allocateSubcontext(
    const Metadata *contextObjectMetadata) {
  auto size = contextObjectMetadata->vw_size();
  auto align = contextObjectMetadata->vw_alignment();
  auto *contextObjectPtr = allocator.Allocate(size, align);
  allocatedContextObjects.push_back(
      AllocatedContextObjectRecord{contextObjectMetadata, contextObjectPtr});
  return contextObjectPtr;
}

AutoDiffLinearMapContext *language::language_autoDiffCreateLinearMapContext(
    size_t topLevelLinearMapStructSize) {
  auto allocationSize = alignTo(
      sizeof(AutoDiffLinearMapContext), alignof(AutoDiffLinearMapContext))
      + topLevelLinearMapStructSize;
  auto *buffer = (AutoDiffLinearMapContext *)malloc(allocationSize);
  return ::new (buffer) AutoDiffLinearMapContext;
}

void *language::language_autoDiffProjectTopLevelSubcontext(
    AutoDiffLinearMapContext *linearMapContext) {
  return static_cast<void *>(linearMapContext->projectTopLevelSubcontext());
}

void *language::language_autoDiffAllocateSubcontext(
    AutoDiffLinearMapContext *allocator, size_t size) {
  return allocator->allocate(size);
}

AutoDiffLinearMapContext *language::language_autoDiffCreateLinearMapContextWithType(
    const Metadata *topLevelLinearMapContextMetadata) {
  assert(topLevelLinearMapContextMetadata->getValueWitnesses() != nullptr);
  auto topLevelLinearMapContextSize =
      topLevelLinearMapContextMetadata->vw_size();
  auto allocationSize = alignTo(sizeof(AutoDiffLinearMapContext),
                                alignof(AutoDiffLinearMapContext)) +
                        topLevelLinearMapContextSize;
  auto *buffer = (AutoDiffLinearMapContext *)malloc(allocationSize);
  return ::new (buffer)
      AutoDiffLinearMapContext(topLevelLinearMapContextMetadata);
}

void *language::language_autoDiffAllocateSubcontextWithType(
    AutoDiffLinearMapContext *linearMapContext,
    const Metadata *linearMapSubcontextMetadata) {
  assert(linearMapSubcontextMetadata->getValueWitnesses() != nullptr);
  return linearMapContext->allocateSubcontext(linearMapSubcontextMetadata);
}

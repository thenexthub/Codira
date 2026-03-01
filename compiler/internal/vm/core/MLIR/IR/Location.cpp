//===- Location.cpp - MLIR Location Classes -------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "mlir/IR/Location.h"
#include "mlir/IR/AttributeSupport.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/Hashing.h"
#include "vm/core/ADT/PointerIntPair.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/TrailingObjects.h"
#include <cassert>
#include <tuple>
#include <utility>

using namespace mlir;
using namespace mlir::detail;

namespace mlir::detail {
struct FileLineColRangeAttrStorage final
    : public ::mlir::AttributeStorage,
      private toolchain::TrailingObjects<FileLineColRangeAttrStorage, unsigned> {
  friend toolchain::TrailingObjects<FileLineColRangeAttrStorage, unsigned>;
  using PointerPair = toolchain::PointerIntPair<StringAttr, 2>;
  using KeyTy = std::tuple<StringAttr, ::toolchain::ArrayRef<unsigned>>;

  FileLineColRangeAttrStorage(StringAttr filename, int numLocs)
      : filenameAndTrailing(filename, numLocs) {}

  static FileLineColRangeAttrStorage *
  construct(::mlir::AttributeStorageAllocator &allocator, KeyTy &&tblgenKey) {
    auto numInArray = std::get<1>(tblgenKey).size();
    // Note: Considered asserting that numInArray is at least 1, but this
    // is not needed in memory or in printed form. This should very rarely be
    // 0 here as that means a NamedLoc would have been more efficient. But this
    // does allow for location with just a file, and also having the interface
    // be more uniform.
    auto locEnc = numInArray == 0 ? 1 : numInArray;
    // Allocate a new storage instance.
    auto byteSize =
        FileLineColRangeAttrStorage::totalSizeToAlloc<unsigned>(locEnc - 1);
    auto *rawMem =
        allocator.allocate(byteSize, alignof(FileLineColRangeAttrStorage));
    auto *result = ::new (rawMem)
        FileLineColRangeAttrStorage(std::get<0>(tblgenKey), locEnc - 1);
    if (numInArray > 0) {
      ArrayRef<unsigned> elements = std::get<1>(tblgenKey);
      result->startLine = elements[0];
      // Copy in the element types into the trailing storage.
      toolchain::uninitialized_copy(elements.drop_front(),
                               result->getTrailingObjects());
    }
    return result;
  }

  // Return the number of held types.
  unsigned size() const { return filenameAndTrailing.getInt() + 1; }

  bool operator==(const KeyTy &tblgenKey) const {
    return (filenameAndTrailing.getPointer() == std::get<0>(tblgenKey)) &&
           (size() == std::get<1>(tblgenKey).size()) &&
           (startLine == std::get<1>(tblgenKey)[0]) &&
           (getTrailingObjects(size() - 1) ==
            std::get<1>(tblgenKey).drop_front());
  }

  unsigned getLineCols(unsigned index) const {
    return getTrailingObjects()[index - 1];
  }

  unsigned getStartLine() const { return startLine; }
  unsigned getStartColumn() const {
    if (size() <= 1)
      return 0;
    return getLineCols(1);
  }
  unsigned getEndColumn() const {
    if (size() <= 2)
      return getStartColumn();
    return getLineCols(2);
  }
  unsigned getEndLine() const {
    if (size() <= 3)
      return getStartLine();
    return getLineCols(3);
  }

  static ::toolchain::hash_code hashKey(const KeyTy &tblgenKey) {
    return ::toolchain::hash_combine(std::get<0>(tblgenKey), std::get<1>(tblgenKey));
  }

  // Supports
  //  - 0 (file:line)
  //  - 1 (file:line:col)
  //  - 2 (file:line:start_col to file:line:end_col) and
  //  - 3 (file:start_line:start_col to file:end_line:end_col)
  toolchain::PointerIntPair<StringAttr, 2> filenameAndTrailing;
  unsigned startLine = 0;
};
} // namespace mlir::detail

//===----------------------------------------------------------------------===//
/// Tablegen Attribute Definitions
//===----------------------------------------------------------------------===//

#define GET_ATTRDEF_CLASSES
#include "mlir/IR/BuiltinLocationAttributes.cpp.inc"

//===----------------------------------------------------------------------===//
// LocationAttr
//===----------------------------------------------------------------------===//

WalkResult LocationAttr::walk(function_ref<WalkResult(Location)> walkFn) {
  AttrTypeWalker walker;
  // Walk locations, but skip any other attribute.
  walker.addWalk([&](Attribute attr) {
    if (auto loc = toolchain::dyn_cast<LocationAttr>(attr))
      return walkFn(loc);

    return WalkResult::skip();
  });
  return walker.walk<WalkOrder::PreOrder>(*this);
}

/// Methods for support type inquiry through isa, cast, and dyn_cast.
bool LocationAttr::classof(Attribute attr) {
  return attr.hasTrait<AttributeTrait::IsLocation>();
}

//===----------------------------------------------------------------------===//
// CallSiteLoc
//===----------------------------------------------------------------------===//

CallSiteLoc CallSiteLoc::get(Location name, ArrayRef<Location> frames) {
  assert(!frames.empty() && "required at least 1 call frame");
  Location caller = frames.back();
  for (auto frame : toolchain::reverse(frames.drop_back()))
    caller = CallSiteLoc::get(frame, caller);
  return CallSiteLoc::get(name, caller);
}

//===----------------------------------------------------------------------===//
// FileLineColLoc
//===----------------------------------------------------------------------===//

FileLineColLoc FileLineColLoc::get(StringAttr filename, unsigned line,
                                   unsigned column) {
  return toolchain::cast<FileLineColLoc>(
      FileLineColRange::get(filename, line, column));
}

FileLineColLoc FileLineColLoc::get(MLIRContext *context, StringRef fileName,
                                   unsigned line, unsigned column) {
  return toolchain::cast<FileLineColLoc>(
      FileLineColRange::get(context, fileName, line, column));
}

StringAttr FileLineColLoc::getFilename() const {
  return FileLineColRange::getFilename();
}

unsigned FileLineColLoc::getLine() const { return getStartLine(); }

unsigned FileLineColLoc::getColumn() const { return getStartColumn(); }

bool mlir::isStrictFileLineColLoc(Location loc) {
  if (auto range = mlir::dyn_cast<FileLineColRange>(loc))
    return range.getImpl()->size() == 2;
  return false;
}

//===----------------------------------------------------------------------===//
// FileLineColRange
//===----------------------------------------------------------------------===//

StringAttr FileLineColRange::getFilename() const {
  return getImpl()->filenameAndTrailing.getPointer();
}

unsigned FileLineColRange::getStartLine() const {
  return getImpl()->getStartLine();
}
unsigned FileLineColRange::getStartColumn() const {
  return getImpl()->getStartColumn();
}
unsigned FileLineColRange::getEndColumn() const {
  return getImpl()->getEndColumn();
}
unsigned FileLineColRange::getEndLine() const {
  return getImpl()->getEndLine();
}

//===----------------------------------------------------------------------===//
// FusedLoc
//===----------------------------------------------------------------------===//

Location FusedLoc::get(ArrayRef<Location> locs, Attribute metadata,
                       MLIRContext *context) {
  // Unique the set of locations to be fused.
  toolchain::SmallSetVector<Location, 4> decomposedLocs;
  for (auto loc : locs) {
    // If the location is a fused location we decompose it if it has no
    // metadata or the metadata is the same as the top level metadata.
    if (auto fusedLoc = toolchain::dyn_cast<FusedLoc>(loc)) {
      if (fusedLoc.getMetadata() == metadata) {
        // UnknownLoc's have already been removed from FusedLocs so we can
        // simply add all of the internal locations.
        decomposedLocs.insert_range(fusedLoc.getLocations());
        continue;
      }
    }
    // Otherwise, only add known locations to the set.
    if (!toolchain::isa<UnknownLoc>(loc))
      decomposedLocs.insert(loc);
  }
  locs = decomposedLocs.getArrayRef();

  // Handle the simple cases of less than two locations. Ensure the metadata (if
  // provided) is not dropped.
  if (locs.empty()) {
    if (!metadata)
      return UnknownLoc::get(context);
    // TODO: Investigate ASAN failure when using implicit conversion from
    // Location to ArrayRef<Location> below.
    return Base::get(context, ArrayRef<Location>{UnknownLoc::get(context)},
                     metadata);
  }
  if (locs.size() == 1 && !metadata)
    return locs.front();

  return Base::get(context, locs, metadata);
}

//===----------------------------------------------------------------------===//
// BuiltinDialect
//===----------------------------------------------------------------------===//

void BuiltinDialect::registerLocationAttributes() {
  addAttributes<
#define GET_ATTRDEF_LIST
#include "mlir/IR/BuiltinLocationAttributes.cpp.inc"
      >();
}

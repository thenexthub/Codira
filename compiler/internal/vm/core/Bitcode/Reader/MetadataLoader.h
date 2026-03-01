//===-- Bitcode/Reader/MetadataLoader.h - Load Metadatas -------*- C++ -*-====//
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
//
// This class handles loading Metadatas.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_BITCODE_READER_METADATALOADER_H
#define LLVM_LIB_BITCODE_READER_METADATALOADER_H

#include "vm/core/Support/Error.h"

#include <functional>
#include <memory>

namespace vm::core {
class BasicBlock;
class BitcodeReaderValueList;
class BitstreamCursor;
class DISubprogram;
class Function;
class Instruction;
class Metadata;
class Module;
class Type;
template <typename T> class ArrayRef;

typedef std::function<Type *(unsigned)> GetTypeByIDTy;

typedef std::function<unsigned(unsigned, unsigned)> GetContainedTypeIDTy;

typedef std::function<void(Metadata **, unsigned, GetTypeByIDTy,
                           GetContainedTypeIDTy)>
    MDTypeCallbackTy;

struct MetadataLoaderCallbacks {
  GetTypeByIDTy GetTypeByID;
  GetContainedTypeIDTy GetContainedTypeID;
  std::optional<MDTypeCallbackTy> MDType;
};

/// Helper class that handles loading Metadatas and keeping them available.
class MetadataLoader {
  class MetadataLoaderImpl;
  std::unique_ptr<MetadataLoaderImpl> Pimpl;
  Error parseMetadata(bool ModuleLevel);

public:
  ~MetadataLoader();
  MetadataLoader(BitstreamCursor &Stream, Module &TheModule,
                 BitcodeReaderValueList &ValueList, bool IsImporting,
                 MetadataLoaderCallbacks Callbacks);
  MetadataLoader &operator=(MetadataLoader &&);
  MetadataLoader(MetadataLoader &&);

  // Parse a module metadata block
  Error parseModuleMetadata() { return parseMetadata(true); }

  // Parse a function metadata block
  Error parseFunctionMetadata() { return parseMetadata(false); }

  /// Set the mode to strip TBAA metadata on load.
  void setStripTBAA(bool StripTBAA = true);

  /// Return true if the Loader is stripping TBAA metadata.
  bool isStrippingTBAA();

  // Return true there are remaining unresolved forward references.
  bool hasFwdRefs() const;

  /// Return the given metadata, creating a replaceable forward reference if
  /// necessary.
  Metadata *getMetadataFwdRefOrLoad(unsigned Idx);

  /// Return the DISubprogram metadata for a Function if any, null otherwise.
  DISubprogram *lookupSubprogramForFunction(Function *F);

  /// Parse a `METADATA_ATTACHMENT` block for a function.
  Error parseMetadataAttachment(Function &F,
                                ArrayRef<Instruction *> InstructionList);

  /// Parse a `METADATA_KIND` block for the current module.
  Error parseMetadataKinds();

  unsigned size() const;
  void shrinkTo(unsigned N);

  /// Perform bitcode upgrades on toolchain.dbg.* calls.
  void upgradeDebugIntrinsics(Function &F);
};
}

#endif // LLVM_LIB_BITCODE_READER_METADATALOADER_H

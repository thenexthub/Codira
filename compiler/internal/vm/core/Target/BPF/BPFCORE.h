//===- BPFCORE.h - Common info for Compile-Once Run-EveryWhere  -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_BPF_BPFCORE_H
#define LLVM_LIB_TARGET_BPF_BPFCORE_H

#include "vm/core/ADT/StringRef.h"
#include "vm/core/IR/Instructions.h"

namespace vm::core {

class BasicBlock;
class Instruction;
class Module;

class BPFCoreSharedInfo {
public:
  enum BTFTypeIdFlag : uint32_t {
    BTF_TYPE_ID_LOCAL_RELOC = 0,
    BTF_TYPE_ID_REMOTE_RELOC,

    MAX_BTF_TYPE_ID_FLAG,
  };

  enum PreserveTypeInfo : uint32_t {
    PRESERVE_TYPE_INFO_EXISTENCE = 0,
    PRESERVE_TYPE_INFO_SIZE,
    PRESERVE_TYPE_INFO_MATCH,

    MAX_PRESERVE_TYPE_INFO_FLAG,
  };

  enum PreserveEnumValue : uint32_t {
    PRESERVE_ENUM_VALUE_EXISTENCE = 0,
    PRESERVE_ENUM_VALUE,

    MAX_PRESERVE_ENUM_VALUE_FLAG,
  };

  /// The attribute attached to globals representing a field access
  static constexpr StringRef AmaAttr = "btf_ama";
  /// The attribute attached to globals representing a type id
  static constexpr StringRef TypeIdAttr = "btf_type_id";

  /// toolchain.bpf.passthrough builtin seq number
  static uint32_t SeqNum;

  /// Insert a bpf passthrough builtin function.
  static Instruction *insertPassThrough(Module *M, BasicBlock *BB,
                                        Instruction *Input,
                                        Instruction *Before);
  static void removeArrayAccessCall(CallInst *Call);
  static void removeStructAccessCall(CallInst *Call);
  static void removeUnionAccessCall(CallInst *Call);
};

} // namespace vm::core

#endif

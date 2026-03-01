//===- InputElement.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLD_WASM_INPUT_ELEMENT_H
#define LLD_WASM_INPUT_ELEMENT_H

#include "Config.h"
#include "InputFiles.h"
#include "WriterUtils.h"
#include "lld/Common/LLVM.h"
#include "llvm/Object/Wasm.h"
#include <optional>

namespace lld {
namespace wasm {

// Represents a single element (Global, Tag, Table, etc) within an input
// file.
class InputElement {
protected:
  InputElement(StringRef name, ObjFile *f)
      : file(f), live(!ctx.arg.gcSections), name(name) {}

public:
  StringRef getName() const { return name; }
  uint32_t getAssignedIndex() const { return *assignedIndex; }
  bool hasAssignedIndex() const { return assignedIndex.has_value(); }
  void assignIndex(uint32_t index) {
    assert(!hasAssignedIndex());
    assignedIndex = index;
  }

  ObjFile *file;
  bool live = false;

protected:
  StringRef name;
  std::optional<uint32_t> assignedIndex;
};

inline WasmInitExpr intConst(uint64_t value, bool is64) {
  WasmInitExpr ie;
  ie.Extended = false;
  if (is64) {
    ie.Inst.Opcode = llvm::wasm::WASM_OPCODE_I64_CONST;
    ie.Inst.Value.Int64 = static_cast<int64_t>(value);
  } else {
    ie.Inst.Opcode = llvm::wasm::WASM_OPCODE_I32_CONST;
    ie.Inst.Value.Int32 = static_cast<int32_t>(value);
  }
  return ie;
}

class InputGlobal : public InputElement {
public:
  InputGlobal(const WasmGlobal &g, ObjFile *f)
      : InputElement(g.SymbolName, f), type(g.Type), initExpr(g.InitExpr) {}

  const WasmGlobalType &getType() const { return type; }
  const WasmInitExpr &getInitExpr() const { return initExpr; }

  void setPointerValue(uint64_t value) {
    initExpr = intConst(value, ctx.arg.is64.value_or(false));
  }

private:
  WasmGlobalType type;
  WasmInitExpr initExpr;
};

class InputTag : public InputElement {
public:
  InputTag(const WasmSignature &s, const WasmTag &t, ObjFile *f)
      : InputElement(t.SymbolName, f), signature(s) {
    assert(s.Kind == WasmSignature::Tag);
  }

  const WasmSignature &signature;
};

class InputTable : public InputElement {
public:
  InputTable(const WasmTable &t, ObjFile *f)
      : InputElement(t.SymbolName, f), type(t.Type) {}

  const WasmTableType &getType() const { return type; }
  void setLimits(const WasmLimits &limits) { type.Limits = limits; }

private:
  WasmTableType type;
};

} // namespace wasm

inline std::string toString(const wasm::InputElement *d) {
  return (toString(d->file) + ":(" + d->getName() + ")").str();
}

} // namespace lld

#endif // LLD_WASM_INPUT_ELEMENT_H

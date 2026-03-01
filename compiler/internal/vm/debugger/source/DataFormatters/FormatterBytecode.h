//===-- FormatterBytecode.h -------------------------------------*- C++ -*-===//
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

#include "lldb/DataFormatters/TypeSummary.h"
#include "lldb/Symbol/CompilerType.h"

namespace lldb_private {

namespace FormatterBytecode {

enum DataType : uint8_t { Any, String, Int, UInt, Object, Type, Selector };

enum OpCodes : uint8_t {
#define DEFINE_OPCODE(OP, MNEMONIC, NAME) op_##NAME = OP,
#include "FormatterBytecode.def"
#undef DEFINE_OPCODE
};

enum Selectors : uint8_t {
#define DEFINE_SELECTOR(ID, NAME) sel_##NAME = ID,
#include "FormatterBytecode.def"
#undef DEFINE_SELECTOR
};

enum Signatures : uint8_t {
#define DEFINE_SIGNATURE(ID, NAME) sig_##NAME = ID,
#include "FormatterBytecode.def"
#undef DEFINE_SIGNATURE
};

using ControlStackElement = llvm::StringRef;
using DataStackElement =
    std::variant<std::string, uint64_t, int64_t, lldb::ValueObjectSP,
                 CompilerType, Selectors>;
struct DataStack : public std::vector<DataStackElement> {
  DataStack() = default;
  DataStack(lldb::ValueObjectSP initial_value)
      : std::vector<DataStackElement>({initial_value}) {}
  void Push(DataStackElement el) { push_back(el); }
  template <typename T> T Pop() {
    T el = std::get<T>(back());
    pop_back();
    return el;
  }
  DataStackElement PopAny() {
    DataStackElement el = back();
    pop_back();
    return el;
  }
};
llvm::Error Interpret(std::vector<ControlStackElement> &control,
                      DataStack &data, Selectors sel);
} // namespace FormatterBytecode

std::string toString(FormatterBytecode::OpCodes op);
std::string toString(FormatterBytecode::Selectors sel);
std::string toString(FormatterBytecode::Signatures sig);

} // namespace lldb_private

//===-- lib/Semantics/mod-file.h --------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_MOD_FILE_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_MOD_FILE_H_

#include "language/Compability/Semantics/attr.h"
#include "language/Compability/Semantics/symbol.h"
#include "toolchain/Support/raw_ostream.h"
#include <string>

namespace language::Compability::parser {
class CharBlock;
class Message;
class MessageFixedText;
} // namespace language::Compability::parser

namespace toolchain {
class raw_ostream;
}

namespace language::Compability::semantics {

using SourceName = parser::CharBlock;
class Symbol;
class Scope;
class SemanticsContext;

class ModFileWriter {
public:
  explicit ModFileWriter(SemanticsContext &context) : context_{context} {}
  bool WriteAll();
  void WriteClosure(toolchain::raw_ostream &, const Symbol &,
      UnorderedSymbolSet &nonIntrinsicModulesWritten);
  ModFileWriter &set_hermeticModuleFileOutput(bool yes = true) {
    hermeticModuleFileOutput_ = yes;
    return *this;
  }

private:
  SemanticsContext &context_;
  // Buffers to use with raw_string_ostream
  std::string needsBuf_;
  std::string usesBuf_;
  std::string useExtraAttrsBuf_;
  std::string declsBuf_;
  std::string containsBuf_;
  // Tracks nested DEC structures and fields of that type
  UnorderedSymbolSet emittedDECStructures_, emittedDECFields_;
  UnorderedSymbolSet usedNonIntrinsicModules_;

  toolchain::raw_string_ostream needs_{needsBuf_};
  toolchain::raw_string_ostream uses_{usesBuf_};
  toolchain::raw_string_ostream useExtraAttrs_{
      useExtraAttrsBuf_}; // attrs added to used entity
  toolchain::raw_string_ostream decls_{declsBuf_};
  toolchain::raw_string_ostream contains_{containsBuf_};
  bool isSubmodule_{false};
  bool hermeticModuleFileOutput_{false};

  void WriteAll(const Scope &);
  void WriteOne(const Scope &);
  void Write(const Symbol &);
  std::string GetAsString(const Symbol &);
  void PrepareRenamings(const Scope &);
  void PutSymbols(const Scope &, UnorderedSymbolSet *hermetic);
  // Returns true if a derived type with bindings and "contains" was emitted
  bool PutComponents(const Symbol &);
  void PutSymbol(toolchain::raw_ostream &, const Symbol &);
  void PutEntity(toolchain::raw_ostream &, const Symbol &);
  void PutEntity(
      toolchain::raw_ostream &, const Symbol &, std::function<void()>, Attrs);
  void PutObjectEntity(toolchain::raw_ostream &, const Symbol &);
  void PutProcEntity(toolchain::raw_ostream &, const Symbol &);
  void PutDerivedType(const Symbol &, const Scope * = nullptr);
  void PutDECStructure(const Symbol &, const Scope * = nullptr);
  void PutTypeParam(toolchain::raw_ostream &, const Symbol &);
  void PutUserReduction(toolchain::raw_ostream &, const Symbol &);
  void PutSubprogram(const Symbol &);
  void PutGeneric(const Symbol &);
  void PutUse(const Symbol &);
  void PutUseExtraAttr(Attr, const Symbol &, const Symbol &);
  toolchain::raw_ostream &PutAttrs(toolchain::raw_ostream &, Attrs,
      const std::string * = nullptr, bool = false, std::string before = ","s,
      std::string after = ""s) const;
  void PutDirective(toolchain::raw_ostream &, const Symbol &);
};

class ModFileReader {
public:
  ModFileReader(SemanticsContext &context) : context_{context} {}
  // Find and read the module file for a module or submodule.
  // If ancestor is specified, look for a submodule of that module.
  // Return the Scope for that module/submodule or nullptr on error.
  Scope *Read(SourceName, std::optional<bool> isIntrinsic, Scope *ancestor,
      bool silent);

private:
  SemanticsContext &context_;

  parser::Message &Say(const char *verb, SourceName, const std::string &,
      parser::MessageFixedText &&, const std::string &);
};

} // namespace language::Compability::semantics
#endif

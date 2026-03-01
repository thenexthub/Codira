//===- Symbol.cpp ---------------------------------------------------------===//
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
// Implements the Symbol.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TextAPI/Symbol.h"
#include <string>

namespace vm::core {
namespace MachO {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Symbol::dump(raw_ostream &OS) const {
  std::string Result;
  if (isUndefined())
    Result += "(undef) ";
  if (isWeakDefined())
    Result += "(weak-def) ";
  if (isWeakReferenced())
    Result += "(weak-ref) ";
  if (isThreadLocalValue())
    Result += "(tlv) ";
  switch (Kind) {
  case EncodeKind::GlobalSymbol:
    Result += Name.str();
    break;
  case EncodeKind::ObjectiveCClass:
    Result += "(ObjC Class) " + Name.str();
    break;
  case EncodeKind::ObjectiveCClassEHType:
    Result += "(ObjC Class EH) " + Name.str();
    break;
  case EncodeKind::ObjectiveCInstanceVariable:
    Result += "(ObjC IVar) " + Name.str();
    break;
  }
  OS << Result;
}
#endif

Symbol::const_filtered_target_range
Symbol::targets(ArchitectureSet Architectures) const {
  std::function<bool(const Target &)> FN =
      [Architectures](const Target &Target) {
        return Architectures.has(Target.Arch);
      };
  return make_filter_range(Targets, FN);
}

bool Symbol::operator==(const Symbol &O) const {
  // Older Tapi files do not express all these symbol flags. In those
  // cases, ignore those differences.
  auto RemoveFlag = [](const Symbol &Sym, SymbolFlags &Flag) {
    if (Sym.isData())
      Flag &= ~SymbolFlags::Data;
    if (Sym.isText())
      Flag &= ~SymbolFlags::Text;
  };
  SymbolFlags LHSFlags = Flags;
  SymbolFlags RHSFlags = O.Flags;
  // Ignore Text and Data for now.
  RemoveFlag(*this, LHSFlags);
  RemoveFlag(O, RHSFlags);
  return std::tie(Name, Kind, Targets, LHSFlags) ==
         std::tie(O.Name, O.Kind, O.Targets, RHSFlags);
}

SimpleSymbol parseSymbol(StringRef SymName) {
  if (SymName.starts_with(ObjC1ClassNamePrefix))
    return {SymName.drop_front(ObjC1ClassNamePrefix.size()),
            EncodeKind::ObjectiveCClass, ObjCIFSymbolKind::Class};
  if (SymName.starts_with(ObjC2ClassNamePrefix))
    return {SymName.drop_front(ObjC2ClassNamePrefix.size()),
            EncodeKind::ObjectiveCClass, ObjCIFSymbolKind::Class};
  if (SymName.starts_with(ObjC2MetaClassNamePrefix))
    return {SymName.drop_front(ObjC2MetaClassNamePrefix.size()),
            EncodeKind::ObjectiveCClass, ObjCIFSymbolKind::MetaClass};
  if (SymName.starts_with(ObjC2EHTypePrefix))
    return {SymName.drop_front(ObjC2EHTypePrefix.size()),
            EncodeKind::ObjectiveCClassEHType, ObjCIFSymbolKind::EHType};
  if (SymName.starts_with(ObjC2IVarPrefix))
    return {SymName.drop_front(ObjC2IVarPrefix.size()),
            EncodeKind::ObjectiveCInstanceVariable, ObjCIFSymbolKind::None};
  return {SymName, EncodeKind::GlobalSymbol, ObjCIFSymbolKind::None};
}

} // end namespace MachO.
} // end namespace vm::core.

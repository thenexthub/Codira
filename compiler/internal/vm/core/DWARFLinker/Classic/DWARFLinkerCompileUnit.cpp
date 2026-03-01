//===- DWARFLinkerCompileUnit.cpp -----------------------------------------===//
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

#include "vm/core/DWARFLinker/Classic/DWARFLinkerCompileUnit.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/DWARFLinker/Classic/DWARFLinkerDeclContext.h"
#include "vm/core/DebugInfo/DWARF/DWARFContext.h"
#include "vm/core/DebugInfo/DWARF/LowLevel/DWARFExpression.h"
#include "vm/core/Support/FormatVariadic.h"

namespace vm::core {

using namespace dwarf_linker;
using namespace dwarf_linker::classic;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void CompileUnit::DIEInfo::dump() {
  toolchain::errs() << "{\n";
  toolchain::errs() << "  AddrAdjust: " << AddrAdjust << '\n';
  toolchain::errs() << "  Ctxt: " << formatv("{0:x}", Ctxt) << '\n';
  toolchain::errs() << "  Clone: " << formatv("{0:x}", Clone) << '\n';
  toolchain::errs() << "  ParentIdx: " << ParentIdx << '\n';
  toolchain::errs() << "  Keep: " << Keep << '\n';
  toolchain::errs() << "  InDebugMap: " << InDebugMap << '\n';
  toolchain::errs() << "  Prune: " << Prune << '\n';
  toolchain::errs() << "  Incomplete: " << Incomplete << '\n';
  toolchain::errs() << "  InModuleScope: " << InModuleScope << '\n';
  toolchain::errs() << "  ODRMarkingDone: " << ODRMarkingDone << '\n';
  toolchain::errs() << "  UnclonedReference: " << UnclonedReference << '\n';
  toolchain::errs() << "}\n";
}
#endif // if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

/// Check if the DIE at \p Idx is in the scope of a function.
static bool inFunctionScope(CompileUnit &U, unsigned Idx) {
  while (Idx) {
    if (U.getOrigUnit().getDIEAtIndex(Idx).getTag() == dwarf::DW_TAG_subprogram)
      return true;
    Idx = U.getInfo(Idx).ParentIdx;
  }
  return false;
}

uint16_t CompileUnit::getLanguage() {
  if (!Language) {
    DWARFDie CU = getOrigUnit().getUnitDIE();
    Language = dwarf::toUnsigned(CU.find(dwarf::DW_AT_language), 0);
  }
  return Language;
}

StringRef CompileUnit::getSysRoot() {
  if (SysRoot.empty()) {
    DWARFDie CU = getOrigUnit().getUnitDIE();
    SysRoot = dwarf::toStringRef(CU.find(dwarf::DW_AT_LLVM_sysroot)).str();
  }
  return SysRoot;
}

void CompileUnit::markEverythingAsKept() {
  unsigned Idx = 0;

  for (auto &I : Info) {
    // Mark everything that wasn't explicit marked for pruning.
    I.Keep = !I.Prune;
    auto DIE = OrigUnit.getDIEAtIndex(Idx++);
    DWARFUnit *U = DIE.getDwarfUnit();

    // Try to guess which DIEs must go to the accelerator tables. We do that
    // just for variables, because functions will be handled depending on
    // whether they carry a DW_AT_low_pc attribute or not.
    if (DIE.getTag() != dwarf::DW_TAG_variable &&
        DIE.getTag() != dwarf::DW_TAG_constant)
      continue;

    std::optional<DWARFFormValue> Value;
    if (!(Value = DIE.find(dwarf::DW_AT_location))) {
      if ((Value = DIE.find(dwarf::DW_AT_const_value)) &&
          !inFunctionScope(*this, I.ParentIdx))
        I.InDebugMap = true;
      continue;
    }

    if (auto ExprLockBlock = Value->getAsBlock()) {
      // Parse 'exprloc' expression.
      DataExtractor Data(toStringRef(*ExprLockBlock),
                         U->getContext().isLittleEndian(),
                         U->getAddressByteSize());
      DWARFExpression Expression(Data, U->getAddressByteSize(),
                                 U->getFormParams().Format);

      for (DWARFExpression::iterator It = Expression.begin();
           (It != Expression.end()) && !I.InDebugMap; ++It) {
        DWARFExpression::iterator NextIt = It;
        ++NextIt;

        switch (It->getCode()) {
        case dwarf::DW_OP_const2u:
        case dwarf::DW_OP_const4u:
        case dwarf::DW_OP_const8u:
        case dwarf::DW_OP_const2s:
        case dwarf::DW_OP_const4s:
        case dwarf::DW_OP_const8s:
          if (NextIt == Expression.end() ||
              NextIt->getCode() != dwarf::DW_OP_form_tls_address)
            break;
          [[fallthrough]];
        case dwarf::DW_OP_constx:
        case dwarf::DW_OP_addr:
        case dwarf::DW_OP_addrx:
          I.InDebugMap = true;
          break;
        default:
          // Nothing to do.
          break;
        }
      }
    }
  }
}

uint64_t CompileUnit::computeNextUnitOffset(uint16_t DwarfVersion) {
  NextUnitOffset = StartOffset;
  if (NewUnit) {
    NextUnitOffset += (DwarfVersion >= 5) ? 12 : 11; // Header size
    NextUnitOffset += NewUnit->getUnitDie().getSize();
  }
  return NextUnitOffset;
}

/// Keep track of a forward cross-cu reference from this unit
/// to \p Die that lives in \p RefUnit.
void CompileUnit::noteForwardReference(DIE *Die, const CompileUnit *RefUnit,
                                       DeclContext *Ctxt, PatchLocation Attr) {
  ForwardDIEReferences.emplace_back(Die, RefUnit, Ctxt, Attr);
}

void CompileUnit::fixupForwardReferences() {
  for (const auto &Ref : ForwardDIEReferences) {
    DIE *RefDie;
    const CompileUnit *RefUnit;
    PatchLocation Attr;
    DeclContext *Ctxt;
    std::tie(RefDie, RefUnit, Ctxt, Attr) = Ref;
    if (Ctxt && Ctxt->hasCanonicalDIE()) {
      assert(Ctxt->getCanonicalDIEOffset() &&
             "Canonical die offset is not set");
      Attr.set(Ctxt->getCanonicalDIEOffset());
    } else {
      assert(RefDie->getOffset() && "Referenced die offset is not set");
      Attr.set(RefDie->getOffset() + RefUnit->getStartOffset());
    }
  }
}

void CompileUnit::addLabelLowPc(uint64_t LabelLowPc, int64_t PcOffset) {
  Labels.insert({LabelLowPc, PcOffset});
}

void CompileUnit::addFunctionRange(uint64_t FuncLowPc, uint64_t FuncHighPc,
                                   int64_t PcOffset) {
  Ranges.insert({FuncLowPc, FuncHighPc}, PcOffset);
  if (LowPc)
    LowPc = std::min(*LowPc, FuncLowPc + PcOffset);
  else
    LowPc = FuncLowPc + PcOffset;
  this->HighPc = std::max(HighPc, FuncHighPc + PcOffset);
}

void CompileUnit::noteRangeAttribute(const DIE &Die, PatchLocation Attr) {
  if (Die.getTag() == dwarf::DW_TAG_compile_unit) {
    UnitRangeAttribute = Attr;
    return;
  }

  RangeAttributes.emplace_back(Attr);
}

void CompileUnit::noteLocationAttribute(PatchLocation Attr) {
  LocationAttributes.emplace_back(Attr);
}

void CompileUnit::noteStmtSeqListAttribute(PatchLocation Attr) {
  StmtSeqListAttributes.emplace_back(Attr);
}

void CompileUnit::addNamespaceAccelerator(const DIE *Die,
                                          DwarfStringPoolEntryRef Name) {
  Namespaces.emplace_back(Name, Die);
}

void CompileUnit::addObjCAccelerator(const DIE *Die,
                                     DwarfStringPoolEntryRef Name,
                                     bool SkipPubSection) {
  ObjC.emplace_back(Name, Die, SkipPubSection);
}

void CompileUnit::addNameAccelerator(const DIE *Die,
                                     DwarfStringPoolEntryRef Name,
                                     bool SkipPubSection) {
  Pubnames.emplace_back(Name, Die, SkipPubSection);
}

void CompileUnit::addTypeAccelerator(const DIE *Die,
                                     DwarfStringPoolEntryRef Name,
                                     bool ObjcClassImplementation,
                                     uint32_t QualifiedNameHash) {
  Pubtypes.emplace_back(Name, Die, QualifiedNameHash, ObjcClassImplementation);
}

} // namespace vm::core

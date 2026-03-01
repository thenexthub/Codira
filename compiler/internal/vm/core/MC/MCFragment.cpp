//===- lib/MC/MCFragment.cpp - Assembler Fragment Implementation ----------===//
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

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCSection.h"
#include "vm/core/MC/MCSectionMachO.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>
#include <type_traits>
#include <utility>

using namespace vm::core;

static_assert(std::is_trivially_destructible_v<MCFragment>,
              "fragment classes must be trivially destructible");

MCFragment::MCFragment(FragmentType Kind, bool HasInstructions)
    : Kind(Kind), LinkerRelaxable(false), HasInstructions(HasInstructions),
      AllowAutoPadding(false) {
  static_assert(sizeof(MCFragment::Tail) <= 16,
                "Keep the variable-size tail small");
}

const MCSymbol *MCFragment::getAtom() const {
  return static_cast<const MCSectionMachO *>(Parent)->getAtom(LayoutOrder);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void MCFragment::dump() const {
  raw_ostream &OS = errs();

  OS << Offset << ' ';
  switch (getKind()) {
    // clang-format off
  case MCFragment::FT_Align:         OS << "Align"; break;
  case MCFragment::FT_Data:          OS << "Data"; break;
  case MCFragment::FT_Fill:          OS << "Fill"; break;
  case MCFragment::FT_Nops:          OS << "Nops"; break;
  case MCFragment::FT_Relaxable:     OS << "Relaxable"; break;
  case MCFragment::FT_Org:           OS << "Org"; break;
  case MCFragment::FT_Dwarf:         OS << "Dwarf"; break;
  case MCFragment::FT_DwarfFrame:    OS << "DwarfCallFrame"; break;
  case MCFragment::FT_SFrame:        OS << "SFrame"; break;
  case MCFragment::FT_LEB:           OS << "LEB"; break;
  case MCFragment::FT_BoundaryAlign: OS<<"BoundaryAlign"; break;
  case MCFragment::FT_SymbolId:      OS << "SymbolId"; break;
  case MCFragment::FT_CVInlineLines: OS << "CVInlineLineTable"; break;
  case MCFragment::FT_CVDefRange:    OS << "CVDefRangeTable"; break;
    // clang-format on
  }

  auto printFixups = [&](toolchain::ArrayRef<MCFixup> Fixups) {
    if (Fixups.empty())
      return;
    for (auto [I, F] : toolchain::enumerate(Fixups)) {
      OS << "\n  Fixup @" << F.getOffset() << " Value:";
      F.getValue()->print(OS, nullptr);
      OS << " Kind:" << F.getKind();
      if (F.isLinkerRelaxable())
        OS << " LinkerRelaxable";
    }
  };

  switch (getKind()) {
  case MCFragment::FT_Data:
  case MCFragment::FT_Relaxable:
  case MCFragment::FT_Align:
  case MCFragment::FT_LEB:
  case MCFragment::FT_Dwarf:
  case MCFragment::FT_DwarfFrame:
  case MCFragment::FT_SFrame: {
    if (isLinkerRelaxable())
      OS << " LinkerRelaxable";
    auto Fixed = getContents();
    auto Var = getVarContents();
    OS << " Size:" << Fixed.size();
    if (getKind() != MCFragment::FT_Data) {
      OS << '+' << Var.size();
      // FT_Align uses getVarContents to track the size, but the content is
      // ignored and not useful.
      if (getKind() == MCFragment::FT_Align)
        Var = {};
    }
    OS << " [";
    for (unsigned i = 0, e = Fixed.size(); i != e; ++i) {
      if (i) OS << ",";
      OS << format("%02x", uint8_t(Fixed[i]));
    }
    for (unsigned i = 0, e = Var.size(); i != e; ++i) {
      if (Fixed.size() || i)
        OS << ",";
      OS << format("%02x", uint8_t(Var[i]));
    }
    OS << ']';
    switch (getKind()) {
    case MCFragment::FT_Data:
      break;
    case MCFragment::FT_Relaxable:
      OS << ' ';
      getInst().dump_pretty(OS);
      break;
    case MCFragment::FT_Align:
      OS << "\n  Align:" << getAlignment().value() << " Fill:" << getAlignFill()
         << " FillLen:" << unsigned(getAlignFillLen())
         << " MaxBytesToEmit:" << getAlignMaxBytesToEmit();
      if (hasAlignEmitNops())
        OS << " Nops";
      break;
    case MCFragment::FT_LEB: {
      OS << " Value:";
      getLEBValue().print(OS, nullptr);
      OS << " Signed:" << isLEBSigned();
      break;
    }
    case MCFragment::FT_Dwarf:
      OS << " AddrDelta:";
      getDwarfAddrDelta().print(OS, nullptr);
      OS << " LineDelta:" << getDwarfLineDelta();
      break;
    case MCFragment::FT_DwarfFrame:
    case MCFragment::FT_SFrame:
      OS << " AddrDelta:";
      getDwarfAddrDelta().print(OS, nullptr);
      break;
    default:
      llvm_unreachable("");
    }
    printFixups(getFixups());
    printFixups(getVarFixups());
    break;
  }
  case MCFragment::FT_Fill:  {
    const auto *FF = cast<MCFillFragment>(this);
    OS << " Value:" << static_cast<unsigned>(FF->getValue())
       << " ValueSize:" << static_cast<unsigned>(FF->getValueSize())
       << " NumValues:";
    FF->getNumValues().print(OS, nullptr);
    break;
  }
  case MCFragment::FT_Nops: {
    const auto *NF = cast<MCNopsFragment>(this);
    OS << " NumBytes:" << NF->getNumBytes()
       << " ControlledNopLength:" << NF->getControlledNopLength();
    break;
  }
  case MCFragment::FT_Org:  {
    const auto *OF = cast<MCOrgFragment>(this);
    OS << " Offset:";
    OF->getOffset().print(OS, nullptr);
    OS << " Value:" << static_cast<unsigned>(OF->getValue());
    break;
  }
  case MCFragment::FT_BoundaryAlign: {
    const auto *BF = cast<MCBoundaryAlignFragment>(this);
    OS << " BoundarySize:" << BF->getAlignment().value()
       << " LastFragment:" << BF->getLastFragment()
       << " Size:" << BF->getSize();
    break;
  }
  case MCFragment::FT_SymbolId: {
    const auto *F = cast<MCSymbolIdFragment>(this);
    OS << " Sym:" << F->getSymbol();
    break;
  }
  case MCFragment::FT_CVInlineLines: {
    const auto *F = cast<MCCVInlineLineTableFragment>(this);
    OS << " Sym:" << *F->getFnStartSym();
    break;
  }
  case MCFragment::FT_CVDefRange: {
    const auto *F = cast<MCCVDefRangeFragment>(this);
    OS << "\n   ";
    for (std::pair<const MCSymbol *, const MCSymbol *> RangeStartEnd :
         F->getRanges()) {
      OS << " RangeStart:" << RangeStartEnd.first;
      OS << " RangeEnd:" << RangeStartEnd.second;
    }
    break;
  }
  }
}
#endif

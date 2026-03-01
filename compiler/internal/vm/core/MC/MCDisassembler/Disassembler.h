//===------------- Disassembler.h - LLVM Disassembler -----------*- C++ -*-===//
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
// This file defines the interface for the Disassembly library's disassembler
// context.  The disassembler is responsible for producing strings for
// individual instructions according to a given architecture and disassembly
// syntax.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_MC_MCDISASSEMBLER_DISASSEMBLER_H
#define LLVM_LIB_MC_MCDISASSEMBLER_DISASSEMBLER_H

#include "vm/core-c/DisassemblerTypes.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCDisassembler/MCDisassembler.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/Support/raw_ostream.h"
#include <string>
#include <utility>

namespace vm::core {
class Target;

//
// This is the disassembler context returned by LLVMCreateDisasm().
//
class LLVMDisasmContext {
private:
  //
  // The passed parameters when the disassembler context is created.
  //
  // The TripleName for this disassembler.
  std::string TripleName;
  // The pointer to the caller's block of symbolic information.
  void *DisInfo;
  // The Triple specific symbolic information type returned by GetOpInfo.
  int TagType;
  // The function to get the symbolic information for operands.
  LLVMOpInfoCallback GetOpInfo;
  // The function to look up a symbol name.
  LLVMSymbolLookupCallback SymbolLookUp;
  //
  // The objects created and saved by LLVMCreateDisasm() then used by
  // LLVMDisasmInstruction().
  //
  // The LLVM target corresponding to the disassembler.
  // FIXME: using std::unique_ptr<const toolchain::Target> causes a malloc error
  //        when this LLVMDisasmContext is deleted.
  const Target *TheTarget;
  // The assembly information for the target architecture.
  std::unique_ptr<const toolchain::MCAsmInfo> MAI;
  // The register information for the target architecture.
  std::unique_ptr<const toolchain::MCRegisterInfo> MRI;
  // The subtarget information for the target architecture.
  std::unique_ptr<const toolchain::MCSubtargetInfo> MSI;
  // The instruction information for the target architecture.
  std::unique_ptr<const toolchain::MCInstrInfo> MII;
  // The assembly context for creating symbols and MCExprs.
  std::unique_ptr<const toolchain::MCContext> Ctx;
  // The disassembler for the target architecture.
  std::unique_ptr<const toolchain::MCDisassembler> DisAsm;
  // The instruction printer for the target architecture.
  std::unique_ptr<toolchain::MCInstPrinter> IP;
  // The options used to set up the disassembler.
  uint64_t Options;
  // The CPU string.
  std::string CPU;

public:
  // Comment stream and backing vector.
  SmallString<128> CommentsToEmit;
  raw_svector_ostream CommentStream;

  LLVMDisasmContext(std::string TripleName, void *DisInfo, int TagType,
                    LLVMOpInfoCallback GetOpInfo,
                    LLVMSymbolLookupCallback SymbolLookUp,
                    const Target *TheTarget,
                    std::unique_ptr<const MCAsmInfo> &&MAI,
                    std::unique_ptr<const MCRegisterInfo> &&MRI,
                    std::unique_ptr<const MCSubtargetInfo> &&MSI,
                    std::unique_ptr<const MCInstrInfo> &&MII,
                    std::unique_ptr<const toolchain::MCContext> &&Ctx,
                    std::unique_ptr<const MCDisassembler> &&DisAsm,
                    std::unique_ptr<MCInstPrinter> &&IP)
      : TripleName(std::move(TripleName)), DisInfo(DisInfo), TagType(TagType),
        GetOpInfo(GetOpInfo), SymbolLookUp(SymbolLookUp), TheTarget(TheTarget),
        MAI(std::move(MAI)), MRI(std::move(MRI)), MSI(std::move(MSI)),
        MII(std::move(MII)), Ctx(std::move(Ctx)), DisAsm(std::move(DisAsm)),
        IP(std::move(IP)), Options(0), CommentStream(CommentsToEmit) {}
  StringRef getTripleName() const { return TripleName; }
  void *getDisInfo() const { return DisInfo; }
  int getTagType() const { return TagType; }
  LLVMOpInfoCallback getGetOpInfo() const { return GetOpInfo; }
  LLVMSymbolLookupCallback getSymbolLookupCallback() const {
    return SymbolLookUp;
  }
  const Target *getTarget() const { return TheTarget; }
  const MCDisassembler *getDisAsm() const { return DisAsm.get(); }
  const MCAsmInfo *getAsmInfo() const { return MAI.get(); }
  const MCInstrInfo *getInstrInfo() const { return MII.get(); }
  const MCRegisterInfo *getRegisterInfo() const { return MRI.get(); }
  const MCSubtargetInfo *getSubtargetInfo() const { return MSI.get(); }
  MCInstPrinter *getIP() { return IP.get(); }
  void setIP(MCInstPrinter *NewIP) { IP.reset(NewIP); }
  uint64_t getOptions() const { return Options; }
  void addOptions(uint64_t Options) { this->Options |= Options; }
  StringRef getCPU() const { return CPU; }
  void setCPU(const char *CPU) { this->CPU = CPU; }
};

} // namespace vm::core

#endif

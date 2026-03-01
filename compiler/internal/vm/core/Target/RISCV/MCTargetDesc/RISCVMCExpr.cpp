//===-- RISCVMCExpr.cpp - RISC-V specific MC expression classes -----------===//
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
// This file contains the implementation of the assembly expression modifiers
// accepted by the RISC-V architecture (e.g. ":lo12:", ":gottprel_g1:", ...).
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/RISCVAsmBackend.h"
#include "MCTargetDesc/RISCVMCAsmInfo.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

#define DEBUG_TYPE "riscvmcexpr"

RISCV::Specifier RISCV::parseSpecifierName(StringRef name) {
  return StringSwitch<RISCV::Specifier>(name)
      .Case("lo", RISCV::S_LO)
      .Case("hi", ELF::R_RISCV_HI20)
      .Case("pcrel_lo", RISCV::S_PCREL_LO)
      .Case("pcrel_hi", ELF::R_RISCV_PCREL_HI20)
      .Case("got_pcrel_hi", ELF::R_RISCV_GOT_HI20)
      .Case("tprel_lo", RISCV::S_TPREL_LO)
      .Case("tprel_hi", ELF::R_RISCV_TPREL_HI20)
      .Case("tprel_add", ELF::R_RISCV_TPREL_ADD)
      .Case("tls_ie_pcrel_hi", ELF::R_RISCV_TLS_GOT_HI20)
      .Case("tls_gd_pcrel_hi", ELF::R_RISCV_TLS_GD_HI20)
      .Case("tlsdesc_hi", ELF::R_RISCV_TLSDESC_HI20)
      .Case("tlsdesc_load_lo", ELF::R_RISCV_TLSDESC_LOAD_LO12)
      .Case("tlsdesc_add_lo", ELF::R_RISCV_TLSDESC_ADD_LO12)
      .Case("tlsdesc_call", ELF::R_RISCV_TLSDESC_CALL)
      .Case("qc.abs20", RISCV::S_QC_ABS20)
      // Used in data directives
      .Case("pltpcrel", ELF::R_RISCV_PLT32)
      .Case("gotpcrel", ELF::R_RISCV_GOT32_PCREL)
      .Default(0);
}

StringRef RISCV::getSpecifierName(Specifier S) {
  switch (S) {
  case RISCV::S_None:
    llvm_unreachable("not used as %specifier()");
  case RISCV::S_LO:
    return "lo";
  case ELF::R_RISCV_HI20:
    return "hi";
  case RISCV::S_PCREL_LO:
    return "pcrel_lo";
  case ELF::R_RISCV_PCREL_HI20:
    return "pcrel_hi";
  case ELF::R_RISCV_GOT_HI20:
    return "got_pcrel_hi";
  case RISCV::S_TPREL_LO:
    return "tprel_lo";
  case ELF::R_RISCV_TPREL_HI20:
    return "tprel_hi";
  case ELF::R_RISCV_TPREL_ADD:
    return "tprel_add";
  case ELF::R_RISCV_TLS_GOT_HI20:
    return "tls_ie_pcrel_hi";
  case ELF::R_RISCV_TLSDESC_HI20:
    return "tlsdesc_hi";
  case ELF::R_RISCV_TLSDESC_LOAD_LO12:
    return "tlsdesc_load_lo";
  case ELF::R_RISCV_TLSDESC_ADD_LO12:
    return "tlsdesc_add_lo";
  case ELF::R_RISCV_TLSDESC_CALL:
    return "tlsdesc_call";
  case ELF::R_RISCV_TLS_GD_HI20:
    return "tls_gd_pcrel_hi";
  case ELF::R_RISCV_CALL_PLT:
    return "call_plt";
  case ELF::R_RISCV_32_PCREL:
    return "32_pcrel";
  case ELF::R_RISCV_GOT32_PCREL:
    return "gotpcrel";
  case ELF::R_RISCV_PLT32:
    return "pltpcrel";
  case RISCV::S_QC_ABS20:
    return "qc.abs20";
  }
  llvm_unreachable("Invalid ELF symbol kind");
}

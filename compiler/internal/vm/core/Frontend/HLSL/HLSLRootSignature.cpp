//===- HLSLRootSignature.cpp - HLSL Root Signature helpers ----------------===//
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
///
/// \file This file contains helpers for working with HLSL Root Signatures.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Frontend/HLSL/HLSLRootSignature.h"
#include "vm/core/Support/DXILABI.h"
#include "vm/core/Support/InterleavedRange.h"
#include "vm/core/Support/ScopedPrinter.h"

namespace vm::core {
namespace hlsl {
namespace rootsig {

template <typename T>
static raw_ostream &printFlags(raw_ostream &OS, const T Value,
                               ArrayRef<EnumEntry<T>> Flags) {
  bool FlagSet = false;
  unsigned Remaining = toolchain::to_underlying(Value);
  while (Remaining) {
    unsigned Bit = 1u << toolchain::countr_zero(Remaining);
    if (Remaining & Bit) {
      if (FlagSet)
        OS << " | ";

      StringRef MaybeFlag = enumToStringRef(T(Bit), Flags);
      if (!MaybeFlag.empty())
        OS << MaybeFlag;
      else
        OS << "invalid: " << Bit;

      FlagSet = true;
    }
    Remaining &= ~Bit;
  }

  if (!FlagSet)
    OS << "None";
  return OS;
}

static const EnumEntry<RegisterType> RegisterNames[] = {
    {"b", RegisterType::BReg},
    {"t", RegisterType::TReg},
    {"u", RegisterType::UReg},
    {"s", RegisterType::SReg},
};

static raw_ostream &operator<<(raw_ostream &OS, const Register &Reg) {
  OS << enumToStringRef(Reg.ViewType, ArrayRef(RegisterNames)) << Reg.Number;

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const toolchain::dxbc::ShaderVisibility &Visibility) {
  OS << enumToStringRef(Visibility, dxbc::getShaderVisibility());

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const toolchain::dxbc::SamplerFilter &Filter) {
  OS << enumToStringRef(Filter, dxbc::getSamplerFilters());

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const dxbc::TextureAddressMode &Address) {
  OS << enumToStringRef(Address, dxbc::getTextureAddressModes());

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const dxbc::ComparisonFunc &CompFunc) {
  OS << enumToStringRef(CompFunc, dxbc::getComparisonFuncs());

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const dxbc::StaticBorderColor &BorderColor) {
  OS << enumToStringRef(BorderColor, dxbc::getStaticBorderColors());

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const dxil::ResourceClass &Type) {
  OS << dxil::getResourceClassName(Type);
  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const dxbc::RootDescriptorFlags &Flags) {
  printFlags(OS, Flags, dxbc::getRootDescriptorFlags());

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const toolchain::dxbc::DescriptorRangeFlags &Flags) {
  printFlags(OS, Flags, dxbc::getDescriptorRangeFlags());

  return OS;
}

static raw_ostream &operator<<(raw_ostream &OS,
                               const toolchain::dxbc::StaticSamplerFlags &Flags) {
  printFlags(OS, Flags, dxbc::getStaticSamplerFlags());

  return OS;
}

raw_ostream &operator<<(raw_ostream &OS, const dxbc::RootFlags &Flags) {
  OS << "RootFlags(";
  printFlags(OS, Flags, dxbc::getRootFlags());
  OS << ")";

  return OS;
}

raw_ostream &operator<<(raw_ostream &OS, const RootConstants &Constants) {
  OS << "RootConstants(num32BitConstants = " << Constants.Num32BitConstants
     << ", " << Constants.Reg << ", space = " << Constants.Space
     << ", visibility = " << Constants.Visibility << ")";

  return OS;
}

raw_ostream &operator<<(raw_ostream &OS, const DescriptorTable &Table) {
  OS << "DescriptorTable(numClauses = " << Table.NumClauses
     << ", visibility = " << Table.Visibility << ")";

  return OS;
}

raw_ostream &operator<<(raw_ostream &OS, const DescriptorTableClause &Clause) {
  OS << Clause.Type << "(" << Clause.Reg << ", numDescriptors = ";
  if (Clause.NumDescriptors == NumDescriptorsUnbounded)
    OS << "unbounded";
  else
    OS << Clause.NumDescriptors;
  OS << ", space = " << Clause.Space << ", offset = ";
  if (Clause.Offset == DescriptorTableOffsetAppend)
    OS << "DescriptorTableOffsetAppend";
  else
    OS << Clause.Offset;
  OS << ", flags = " << Clause.Flags << ")";

  return OS;
}

raw_ostream &operator<<(raw_ostream &OS, const RootDescriptor &Descriptor) {
  OS << "Root" << Descriptor.Type << "(" << Descriptor.Reg
     << ", space = " << Descriptor.Space
     << ", visibility = " << Descriptor.Visibility
     << ", flags = " << Descriptor.Flags << ")";

  return OS;
}

raw_ostream &operator<<(raw_ostream &OS, const StaticSampler &Sampler) {
  OS << "StaticSampler(" << Sampler.Reg << ", filter = " << Sampler.Filter
     << ", addressU = " << Sampler.AddressU
     << ", addressV = " << Sampler.AddressV
     << ", addressW = " << Sampler.AddressW
     << ", mipLODBias = " << Sampler.MipLODBias
     << ", maxAnisotropy = " << Sampler.MaxAnisotropy
     << ", comparisonFunc = " << Sampler.CompFunc
     << ", borderColor = " << Sampler.BorderColor
     << ", minLOD = " << Sampler.MinLOD << ", maxLOD = " << Sampler.MaxLOD
     << ", space = " << Sampler.Space << ", visibility = " << Sampler.Visibility
     << ", flags = " << Sampler.Flags << ")";
  return OS;
}

namespace {

// We use the OverloadVisit with std::visit to ensure the compiler catches if a
// new RootElement variant type is added but it's operator<< isn't handled.
template <class... Ts> struct OverloadedVisit : Ts... {
  using Ts::operator()...;
};
template <class... Ts> OverloadedVisit(Ts...) -> OverloadedVisit<Ts...>;

} // namespace

raw_ostream &operator<<(raw_ostream &OS, const RootElement &Element) {
  const auto Visitor = OverloadedVisit{
      [&OS](const dxbc::RootFlags &Flags) { OS << Flags; },
      [&OS](const RootConstants &Constants) { OS << Constants; },
      [&OS](const RootDescriptor &Descriptor) { OS << Descriptor; },
      [&OS](const DescriptorTableClause &Clause) { OS << Clause; },
      [&OS](const DescriptorTable &Table) { OS << Table; },
      [&OS](const StaticSampler &Sampler) { OS << Sampler; },
  };
  std::visit(Visitor, Element);
  return OS;
}

void dumpRootElements(raw_ostream &OS, ArrayRef<RootElement> Elements) {
  OS << " RootElements" << interleaved(Elements, ", ", "{", "}");
}

} // namespace rootsig
} // namespace hlsl
} // namespace vm::core

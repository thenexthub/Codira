//===- toolchain/MC/DXContainerRootSignature.cpp - RootSignature -*- C++ -*-=======//
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

#include "vm/core/MC/DXContainerRootSignature.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/Support/EndianStream.h"

using namespace vm::core;
using namespace vm::core::mcdxbc;

static uint32_t writePlaceholder(raw_svector_ostream &Stream) {
  const uint32_t DummyValue = std::numeric_limits<uint32_t>::max();
  uint32_t Offset = Stream.tell();
  support::endian::write(Stream, DummyValue, toolchain::endianness::little);
  return Offset;
}

static uint32_t rewriteOffsetToCurrentByte(raw_svector_ostream &Stream,
                                           uint32_t Offset) {
  uint32_t ByteOffset = Stream.tell();
  uint32_t Value = support::endian::byte_swap<uint32_t>(
      ByteOffset, toolchain::endianness::little);
  Stream.pwrite(reinterpret_cast<const char *>(&Value), sizeof(Value), Offset);
  return ByteOffset;
}

size_t RootSignatureDesc::getSize() const {
  uint32_t StaticSamplersOffset = computeStaticSamplersOffset();
  size_t StaticSamplersSize = sizeof(dxbc::RTS0::v1::StaticSampler);
  if (Version > 2)
    StaticSamplersSize = sizeof(dxbc::RTS0::v3::StaticSampler);

  return size_t(StaticSamplersOffset) +
         (StaticSamplersSize * StaticSamplers.size());
}

uint32_t RootSignatureDesc::computeRootParametersOffset() const {
  return sizeof(dxbc::RTS0::v1::RootSignatureHeader);
}

uint32_t RootSignatureDesc::computeStaticSamplersOffset() const {
  uint32_t Offset = computeRootParametersOffset();

  for (const RootParameterInfo &I : ParametersContainer) {
    Offset += sizeof(dxbc::RTS0::v1::RootParameterHeader);
    switch (I.Type) {
    case dxbc::RootParameterType::Constants32Bit:
      Offset += sizeof(dxbc::RTS0::v1::RootConstants);
      break;
    case dxbc::RootParameterType::CBV:
    case dxbc::RootParameterType::SRV:
    case dxbc::RootParameterType::UAV:
      if (Version == 1)
        Offset += sizeof(dxbc::RTS0::v1::RootDescriptor);
      else
        Offset += sizeof(dxbc::RTS0::v2::RootDescriptor);

      break;
    case dxbc::RootParameterType::DescriptorTable:
      const DescriptorTable &Table =
          ParametersContainer.getDescriptorTable(I.Location);

      // 4 bytes for the number of ranges in table and
      // 4 bytes for the ranges offset
      Offset += 2 * sizeof(uint32_t);
      if (Version == 1)
        Offset += sizeof(dxbc::RTS0::v1::DescriptorRange) * Table.Ranges.size();
      else
        Offset += sizeof(dxbc::RTS0::v2::DescriptorRange) * Table.Ranges.size();
      break;
    }
  }

  return Offset;
}

void RootSignatureDesc::write(raw_ostream &OS) const {
  SmallString<256> Storage;
  raw_svector_ostream BOS(Storage);
  BOS.reserveExtraSpace(getSize());

  const uint32_t NumParameters = ParametersContainer.size();
  const uint32_t NumSamplers = StaticSamplers.size();
  support::endian::write(BOS, Version, toolchain::endianness::little);
  support::endian::write(BOS, NumParameters, toolchain::endianness::little);
  support::endian::write(BOS, RootParameterOffset, toolchain::endianness::little);
  support::endian::write(BOS, NumSamplers, toolchain::endianness::little);
  uint32_t SSO = writePlaceholder(BOS);
  support::endian::write(BOS, Flags, toolchain::endianness::little);

  SmallVector<uint32_t> ParamsOffsets;
  for (const RootParameterInfo &I : ParametersContainer) {
    support::endian::write(BOS, I.Type, toolchain::endianness::little);
    support::endian::write(BOS, I.Visibility, toolchain::endianness::little);

    ParamsOffsets.push_back(writePlaceholder(BOS));
  }

  assert(NumParameters == ParamsOffsets.size());
  for (size_t I = 0; I < NumParameters; ++I) {
    rewriteOffsetToCurrentByte(BOS, ParamsOffsets[I]);
    const RootParameterInfo &Info = ParametersContainer.getInfo(I);
    switch (Info.Type) {
    case dxbc::RootParameterType::Constants32Bit: {
      const mcdxbc::RootConstants &Constants =
          ParametersContainer.getConstant(Info.Location);
      support::endian::write(BOS, Constants.ShaderRegister,
                             toolchain::endianness::little);
      support::endian::write(BOS, Constants.RegisterSpace,
                             toolchain::endianness::little);
      support::endian::write(BOS, Constants.Num32BitValues,
                             toolchain::endianness::little);
      break;
    }
    case dxbc::RootParameterType::CBV:
    case dxbc::RootParameterType::SRV:
    case dxbc::RootParameterType::UAV: {
      const mcdxbc::RootDescriptor &Descriptor =
          ParametersContainer.getRootDescriptor(Info.Location);

      support::endian::write(BOS, Descriptor.ShaderRegister,
                             toolchain::endianness::little);
      support::endian::write(BOS, Descriptor.RegisterSpace,
                             toolchain::endianness::little);
      if (Version > 1)
        support::endian::write(BOS, Descriptor.Flags, toolchain::endianness::little);
      break;
    }
    case dxbc::RootParameterType::DescriptorTable: {
      const DescriptorTable &Table =
          ParametersContainer.getDescriptorTable(Info.Location);
      support::endian::write(BOS, (uint32_t)Table.Ranges.size(),
                             toolchain::endianness::little);
      rewriteOffsetToCurrentByte(BOS, writePlaceholder(BOS));
      for (const auto &Range : Table) {
        support::endian::write(BOS, static_cast<uint32_t>(Range.RangeType),
                               toolchain::endianness::little);
        support::endian::write(BOS, Range.NumDescriptors,
                               toolchain::endianness::little);
        support::endian::write(BOS, Range.BaseShaderRegister,
                               toolchain::endianness::little);
        support::endian::write(BOS, Range.RegisterSpace,
                               toolchain::endianness::little);
        if (Version > 1)
          support::endian::write(BOS, Range.Flags, toolchain::endianness::little);
        support::endian::write(BOS, Range.OffsetInDescriptorsFromTableStart,
                               toolchain::endianness::little);
      }
      break;
    }
    }
  }
  [[maybe_unused]] uint32_t Offset = rewriteOffsetToCurrentByte(BOS, SSO);
  assert(Offset == computeStaticSamplersOffset() &&
         "Computed offset does not match written offset");
  for (const auto &S : StaticSamplers) {
    support::endian::write(BOS, S.Filter, toolchain::endianness::little);
    support::endian::write(BOS, S.AddressU, toolchain::endianness::little);
    support::endian::write(BOS, S.AddressV, toolchain::endianness::little);
    support::endian::write(BOS, S.AddressW, toolchain::endianness::little);
    support::endian::write(BOS, S.MipLODBias, toolchain::endianness::little);
    support::endian::write(BOS, S.MaxAnisotropy, toolchain::endianness::little);
    support::endian::write(BOS, S.ComparisonFunc, toolchain::endianness::little);
    support::endian::write(BOS, S.BorderColor, toolchain::endianness::little);
    support::endian::write(BOS, S.MinLOD, toolchain::endianness::little);
    support::endian::write(BOS, S.MaxLOD, toolchain::endianness::little);
    support::endian::write(BOS, S.ShaderRegister, toolchain::endianness::little);
    support::endian::write(BOS, S.RegisterSpace, toolchain::endianness::little);
    support::endian::write(BOS, S.ShaderVisibility, toolchain::endianness::little);

    if (Version > 2)
      support::endian::write(BOS, S.Flags, toolchain::endianness::little);
  }
  assert(Storage.size() == getSize());
  OS.write(Storage.data(), Storage.size());
}

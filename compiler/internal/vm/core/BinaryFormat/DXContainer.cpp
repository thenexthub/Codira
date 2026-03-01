
//===-- toolchain/BinaryFormat/DXContainer.cpp - DXContainer Utils ----*- C++-*-===//
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
// This file contains utility functions for working with DXContainers.
//
//===----------------------------------------------------------------------===//

#include "vm/core/BinaryFormat/DXContainer.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/Support/ScopedPrinter.h"

using namespace vm::core;
using namespace vm::core::dxbc;

#define ROOT_PARAMETER(Val, Enum)                                              \
  case Val:                                                                    \
    return true;
bool toolchain::dxbc::isValidParameterType(uint32_t V) {
  switch (V) {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }
  return false;
}

bool toolchain::dxbc::isValidRangeType(uint32_t V) {
  return V <= toolchain::to_underlying(dxil::ResourceClass::LastEntry);
}

#define SHADER_VISIBILITY(Val, Enum)                                           \
  case Val:                                                                    \
    return true;
bool toolchain::dxbc::isValidShaderVisibility(uint32_t V) {
  switch (V) {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }
  return false;
}

#define FILTER(Val, Enum)                                                      \
  case Val:                                                                    \
    return true;
bool toolchain::dxbc::isValidSamplerFilter(uint32_t V) {
  switch (V) {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }
  return false;
}

#define TEXTURE_ADDRESS_MODE(Val, Enum)                                        \
  case Val:                                                                    \
    return true;
bool toolchain::dxbc::isValidAddress(uint32_t V) {
  switch (V) {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }
  return false;
}

#define COMPARISON_FUNC(Val, Enum)                                             \
  case Val:                                                                    \
    return true;
bool toolchain::dxbc::isValidComparisonFunc(uint32_t V) {
  switch (V) {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }
  return false;
}

#define STATIC_BORDER_COLOR(Val, Enum)                                         \
  case Val:                                                                    \
    return true;
bool toolchain::dxbc::isValidBorderColor(uint32_t V) {
  switch (V) {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }
  return false;
}

bool toolchain::dxbc::isValidRootDesciptorFlags(uint32_t V) {
  using FlagT = dxbc::RootDescriptorFlags;
  uint32_t LargestValue =
      toolchain::to_underlying(FlagT::LLVM_BITMASK_LARGEST_ENUMERATOR);
  return V < NextPowerOf2(LargestValue);
}

bool toolchain::dxbc::isValidDescriptorRangeFlags(uint32_t V) {
  using FlagT = dxbc::DescriptorRangeFlags;
  uint32_t LargestValue =
      toolchain::to_underlying(FlagT::LLVM_BITMASK_LARGEST_ENUMERATOR);
  return V < NextPowerOf2(LargestValue);
}

bool toolchain::dxbc::isValidStaticSamplerFlags(uint32_t V) {
  using FlagT = dxbc::StaticSamplerFlags;
  uint32_t LargestValue =
      toolchain::to_underlying(FlagT::LLVM_BITMASK_LARGEST_ENUMERATOR);
  return V < NextPowerOf2(LargestValue);
}

dxbc::PartType dxbc::parsePartType(StringRef S) {
#define CONTAINER_PART(PartName) .Case(#PartName, PartType::PartName)
  return StringSwitch<dxbc::PartType>(S)
#include "vm/core/BinaryFormat/DXContainerConstants.def"
      .Default(dxbc::PartType::Unknown);
}

bool ShaderHash::isPopulated() {
  static uint8_t Zeros[16] = {0};
  return Flags > 0 || 0 != memcmp(&Digest, &Zeros, 16);
}

#define COMPONENT_PRECISION(Val, Enum) {#Enum, SigMinPrecision::Enum},

static const EnumEntry<SigMinPrecision> SigMinPrecisionNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<SigMinPrecision>> dxbc::getSigMinPrecisions() {
  return ArrayRef(SigMinPrecisionNames);
}

#define D3D_SYSTEM_VALUE(Val, Enum) {#Enum, D3DSystemValue::Enum},

static const EnumEntry<D3DSystemValue> D3DSystemValueNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<D3DSystemValue>> dxbc::getD3DSystemValues() {
  return ArrayRef(D3DSystemValueNames);
}

#define COMPONENT_TYPE(Val, Enum) {#Enum, SigComponentType::Enum},

static const EnumEntry<SigComponentType> SigComponentTypes[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<SigComponentType>> dxbc::getSigComponentTypes() {
  return ArrayRef(SigComponentTypes);
}

static const EnumEntry<RootFlags> RootFlagNames[] = {
#define ROOT_SIGNATURE_FLAG(Val, Enum) {#Enum, RootFlags::Enum},
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<RootFlags>> dxbc::getRootFlags() {
  return ArrayRef(RootFlagNames);
}

static const EnumEntry<RootDescriptorFlags> RootDescriptorFlagNames[] = {
#define ROOT_DESCRIPTOR_FLAG(Val, Enum, Flag)                                  \
  {#Enum, RootDescriptorFlags::Enum},
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<RootDescriptorFlags>> dxbc::getRootDescriptorFlags() {
  return ArrayRef(RootDescriptorFlagNames);
}

static const EnumEntry<DescriptorRangeFlags> DescriptorRangeFlagNames[] = {
#define DESCRIPTOR_RANGE_FLAG(Val, Enum, Flag)                                 \
  {#Enum, DescriptorRangeFlags::Enum},
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<DescriptorRangeFlags>> dxbc::getDescriptorRangeFlags() {
  return ArrayRef(DescriptorRangeFlagNames);
}

static const EnumEntry<StaticSamplerFlags> StaticSamplerFlagNames[] = {
#define STATIC_SAMPLER_FLAG(Val, Enum, Flag) {#Enum, StaticSamplerFlags::Enum},
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<StaticSamplerFlags>> dxbc::getStaticSamplerFlags() {
  return ArrayRef(StaticSamplerFlagNames);
}

#define SHADER_VISIBILITY(Val, Enum) {#Enum, ShaderVisibility::Enum},

static const EnumEntry<ShaderVisibility> ShaderVisibilityValues[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<ShaderVisibility>> dxbc::getShaderVisibility() {
  return ArrayRef(ShaderVisibilityValues);
}

#define FILTER(Val, Enum) {#Enum, SamplerFilter::Enum},

static const EnumEntry<SamplerFilter> SamplerFilterNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<SamplerFilter>> dxbc::getSamplerFilters() {
  return ArrayRef(SamplerFilterNames);
}

#define TEXTURE_ADDRESS_MODE(Val, Enum) {#Enum, TextureAddressMode::Enum},

static const EnumEntry<TextureAddressMode> TextureAddressModeNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<TextureAddressMode>> dxbc::getTextureAddressModes() {
  return ArrayRef(TextureAddressModeNames);
}

#define COMPARISON_FUNC(Val, Enum) {#Enum, ComparisonFunc::Enum},

static const EnumEntry<ComparisonFunc> ComparisonFuncNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<ComparisonFunc>> dxbc::getComparisonFuncs() {
  return ArrayRef(ComparisonFuncNames);
}

#define STATIC_BORDER_COLOR(Val, Enum) {#Enum, StaticBorderColor::Enum},

static const EnumEntry<StaticBorderColor> StaticBorderColorValues[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<StaticBorderColor>> dxbc::getStaticBorderColors() {
  return ArrayRef(StaticBorderColorValues);
}

#define ROOT_PARAMETER(Val, Enum) {#Enum, RootParameterType::Enum},

static const EnumEntry<RootParameterType> RootParameterTypes[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<RootParameterType>> dxbc::getRootParameterTypes() {
  return ArrayRef(RootParameterTypes);
}

#define SEMANTIC_KIND(Val, Enum) {#Enum, PSV::SemanticKind::Enum},

static const EnumEntry<PSV::SemanticKind> SemanticKindNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<PSV::SemanticKind>> PSV::getSemanticKinds() {
  return ArrayRef(SemanticKindNames);
}

#define COMPONENT_TYPE(Val, Enum) {#Enum, PSV::ComponentType::Enum},

static const EnumEntry<PSV::ComponentType> ComponentTypeNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<PSV::ComponentType>> PSV::getComponentTypes() {
  return ArrayRef(ComponentTypeNames);
}

#define INTERPOLATION_MODE(Val, Enum) {#Enum, PSV::InterpolationMode::Enum},

static const EnumEntry<PSV::InterpolationMode> InterpolationModeNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<PSV::InterpolationMode>> PSV::getInterpolationModes() {
  return ArrayRef(InterpolationModeNames);
}

#define RESOURCE_TYPE(Val, Enum) {#Enum, PSV::ResourceType::Enum},

static const EnumEntry<PSV::ResourceType> ResourceTypeNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<PSV::ResourceType>> PSV::getResourceTypes() {
  return ArrayRef(ResourceTypeNames);
}

#define RESOURCE_KIND(Val, Enum) {#Enum, PSV::ResourceKind::Enum},

static const EnumEntry<PSV::ResourceKind> ResourceKindNames[] = {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
};

ArrayRef<EnumEntry<PSV::ResourceKind>> PSV::getResourceKinds() {
  return ArrayRef(ResourceKindNames);
}

//===-- AMDGPUFeatures.td - AMDGPU Feature Definitions -----*- tablegen -*-===//
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

def FeatureFP64 : SubtargetFeature<"fp64",
  "FP64",
  "true",
  "Enable double precision operations"
>;

def FeatureFMA : SubtargetFeature<"fmaf",
  "FMA",
  "true",
  "Enable single precision FMA (not as fast as mul+add, but fused)"
>;

// Addressable local memory size is the maximum number of bytes of LDS that can
// be allocated to a single workgroup.
class SubtargetFeatureAddressableLocalMemorySize <int Value> : SubtargetFeature<
  "addressablelocalmemorysize"#Value,
  "AddressableLocalMemorySize",
  !cast<string>(Value),
  "The size of local memory in bytes"
>;

def FeatureAddressableLocalMemorySize32768 : SubtargetFeatureAddressableLocalMemorySize<32768>;
def FeatureAddressableLocalMemorySize65536 : SubtargetFeatureAddressableLocalMemorySize<65536>;
def FeatureAddressableLocalMemorySize163840 : SubtargetFeatureAddressableLocalMemorySize<163840>;
def FeatureAddressableLocalMemorySize327680 : SubtargetFeatureAddressableLocalMemorySize<327680>;

class SubtargetFeatureWavefrontSize <int ValueLog2> : SubtargetFeature<
  "wavefrontsize"#!shl(1, ValueLog2),
  "WavefrontSizeLog2",
  !cast<string>(ValueLog2),
  "The number of threads per wavefront"
>;

def FeatureWavefrontSize16 : SubtargetFeatureWavefrontSize<4>;
def FeatureWavefrontSize32 : SubtargetFeatureWavefrontSize<5>;
def FeatureWavefrontSize64 : SubtargetFeatureWavefrontSize<6>;

class SubtargetFeatureGeneration <string Value, string FeatureName,
                                 string Subtarget,
                                  list<SubtargetFeature> Implies> :
        SubtargetFeature <FeatureName, "Gen", Subtarget#"::"#Value,
                          Value#" GPU generation", Implies>;

def FeaturePromoteAlloca : SubtargetFeature <"promote-alloca",
  "EnablePromoteAlloca",
  "true",
  "Enable promote alloca pass"
>;


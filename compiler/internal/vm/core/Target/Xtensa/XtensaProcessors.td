//===- XtensaProcessors.td - Xtensa Processors -------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
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

//===----------------------------------------------------------------------===//
// Xtensa supported processors.
//===----------------------------------------------------------------------===//
class Proc<string Name, list<SubtargetFeature> Features>
    : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def : Proc<"esp32", [FeatureDensity, FeatureSingleFloat, FeatureLoop, FeatureMAC16, FeatureWindowed, FeatureBoolean, FeatureSEXT,
                     FeatureNSA, FeatureMul16, FeatureMul32, FeatureMul32High, FeatureDFPAccel, FeatureS32C1I, FeatureTHREADPTR, FeatureDiv32,
                     FeatureDebug, FeatureException, FeatureHighPriInterrupts, FeatureHighPriInterruptsLevel7, FeatureCoprocessor,
                     FeatureInterrupt, FeatureDataCache, FeatureRelocatableVector, FeatureTimers3, FeaturePRID, FeatureRegionProtection, FeatureMiscSR,
                     FeatureMINMAX, FeatureCLAMPS]>;

def : Proc<"esp8266", [FeatureDensity, FeatureNSA, FeatureMul16, FeatureMul32, FeatureExtendedL32R, FeatureDebug, FeatureException,
                       FeatureHighPriInterrupts, FeatureHighPriInterruptsLevel3, FeatureInterrupt, FeatureRelocatableVector, FeatureTimers1,
					   FeatureRegionProtection, FeaturePRID]>;

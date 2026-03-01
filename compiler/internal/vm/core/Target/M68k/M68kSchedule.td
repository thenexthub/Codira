//===-- M68kSchedule.td - M68k Scheduling Definitions ------*- tablegen -*-===//
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
/// \file
/// This file contains M68k scheduler definitions.
///
//===----------------------------------------------------------------------===//

/// This is a very general M68k Scheduling Model and best suited for the very
/// first M68000 CPU, other model must override these characteristics
class M68kSchedModel : SchedMachineModel {
  let LoadLatency = 4;  // Word (Rn)
  let HighLatency = 16; // Long ABS
  let PostRAScheduler = 0;
  let CompleteModel = 0;
}

def GenericM68kModel : M68kSchedModel;

//===-- R600InstrInfo.td - R600 DAG nodes ------------------*- tablegen -*-===//
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
// This file contains DAG node definitions for the R600 target.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// R600 DAG Nodes
//

// Force dependencies for vector trunc stores
def R600dummy_chain : SDNode<"AMDGPUISD::DUMMY_CHAIN", SDTNone, [SDNPHasChain]>;

def R600ExportOp : SDTypeProfile<0, 7, [SDTCisFP<0>, SDTCisInt<1>]>;

def R600_EXPORT: SDNode<"AMDGPUISD::R600_EXPORT", R600ExportOp,
  [SDNPHasChain, SDNPSideEffect]>;

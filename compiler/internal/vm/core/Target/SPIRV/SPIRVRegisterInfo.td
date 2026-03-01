//===-- SPIRVRegisterInfo.td - SPIR-V Register defs --------*- tablegen -*-===//
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
//  Declarations that describe the SPIR-V register file.
//
//===----------------------------------------------------------------------===//

let Namespace = "SPIRV" in {
  // Pointer types for patterns with the GlobalISelEmitter
  def p64 : PtrValueType <i64, 0>;

  class VTPtrVec<int nelem, PtrValueType ptr>
      : VTVec<nelem, ValueType<ptr.Size, ptr.LLVMName>, ptr.LLVMName> {
    int isPointer = true;
  }

  def v2p64 : VTPtrVec<2, p64>;

  // Class for type registers
  def TYPE0 : Register<"TYPE0">;
  def TYPE : RegisterClass<"SPIRV", [i64], 64, (add TYPE0)>;

  // Class for non-type registers
  def ID0 : Register<"ID0">;
  def fID0 : Register<"fID0">;
  def pID0 : Register<"pID0">;
  def vID0 : Register<"vID0">;
  def vfID0 : Register<"vfID0">;
  def vpID0 : Register<"vpID0">;

  def iID : RegisterClass<"SPIRV", [i64], 64, (add ID0)>;
  def fID : RegisterClass<"SPIRV", [f64], 64, (add fID0)>;
  def pID : RegisterClass<"SPIRV", [p64], 64, (add pID0)>;
  def vID : RegisterClass<"SPIRV", [v2i64], 64, (add vID0)>;
  def vfID : RegisterClass<"SPIRV", [v2f64], 64, (add vfID0)>;
  def vpID : RegisterClass<"SPIRV", [v2p64], 64, (add vpID0)>;
  
  def ID : RegisterClass<
      "SPIRV",
      [i64, f64, p64, v2i64, v2f64, v2p64],
      64,
      (add iID, fID, pID, vID, vfID, vpID)>;

  // A few instructions like OpName can take ids from both type and non-type
  // instructions, so we need a super-class to allow for both to count as valid
  // arguments for these instructions.
  def ANY : RegisterClass<"SPIRV", [i64], 64, (add TYPE, ID)>;
}

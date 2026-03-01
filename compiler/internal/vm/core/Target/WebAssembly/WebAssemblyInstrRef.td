// WebAssemblyInstrRef.td - WebAssembly reference type codegen --*- tablegen -*-
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
/// WebAssembly reference type operand codegen constructs.
///
//===----------------------------------------------------------------------===//

multiclass REF_I<WebAssemblyRegClass rc, ValueType vt, string ht> {
  defm REF_NULL_#rc : I<(outs rc:$dst), (ins),
                        (outs), (ins),
                        [(set rc:$dst, (!cast<Intrinsic>("int_wasm_ref_null_" # ht)))],
                        "ref.null_" # ht # "$dst",
                        "ref.null_" # ht,
                        !cond(!eq(ht, "func")   : 0xd070,
                              !eq(ht, "extern") : 0xd06f,
                              !eq(ht, "exn")    : 0xd069)>,
                      Requires<[HasReferenceTypes]>;
  defm SELECT_#rc: I<(outs rc:$dst), (ins rc:$lhs, rc:$rhs, I32:$cond),
                     (outs), (ins),
                     [(set rc:$dst,
                       (select I32:$cond, rc:$lhs, rc:$rhs))],
                     vt#".select\t$dst, $lhs, $rhs, $cond",
                     vt#".select", 0x1b>,
                   Requires<[HasReferenceTypes]>;
  defm REF_IS_NULL_#rc
      : I<(outs I32:$dst), (ins rc:$ref), (outs), (ins),
          [(set I32:$dst, (!cast<Intrinsic>("int_wasm_ref_is_null_" # ht) rc:$ref))],
          "ref.is_null\t$ref",
          "ref.is_null", 0xd1>,
        Requires<[HasReferenceTypes]>;
}

defm REF_TEST_FUNCREF : I<(outs I32:$res), (ins TypeIndex:$type, FUNCREF:$ref),
                          (outs), (ins TypeIndex:$type), [],
                          "ref.test\t$type, $ref", "ref.test $type", 0xfb14>,
                        Requires<[HasGC]>;

defm REF_FUNC : I<(outs FUNCREF:$res), (ins function32_op:$func),
                    (outs), (ins function32_op:$func), [],
                    "ref.func\t$func", "ref.func $func", 0xd2>,
                Requires<[HasReferenceTypes]>;

defm "" : REF_I<FUNCREF, funcref, "func">;
defm "" : REF_I<EXTERNREF, externref, "extern">;
defm "" : REF_I<EXNREF, exnref, "exn">;

foreach rc = [FUNCREF, EXTERNREF, EXNREF] in {
def : Pat<(select (i32 (setne I32:$cond, 0)), rc:$lhs, rc:$rhs),
          (!cast<Instruction>("SELECT_"#rc) rc:$lhs, rc:$rhs, I32:$cond)>;
def : Pat<(select (i32 (seteq I32:$cond, 0)), rc:$lhs, rc:$rhs),
          (!cast<Instruction>("SELECT_"#rc) rc:$rhs, rc:$lhs, I32:$cond)>;
}

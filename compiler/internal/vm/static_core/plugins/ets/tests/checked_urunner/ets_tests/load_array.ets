/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! CHECKER         Load with negative index is correct with 32 bit offset
//! RUN             force_jit: true, options: "--compiler-regex='.*func.*'", entry: "load_array.ETSGLOBAL::main", return: 0
//! METHOD          "load_array.ETSGLOBAL::func"
//! PASS_BEFORE     "AdjustRefs"
//! INST            "LoadArrayI"
//! INST_NEXT       "AddI"
//! INST_NEXT       "LoadArray"
//!
//! PASS_AFTER      "AdjustRefs"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "LoadArrayI"
//! INST_NEXT       is_hybrid_config? ? /AddI.*0x218/ : /AddI.*0x210/
//! INST_NEXT       "Load 8"
//!
//! ASM_METHOD      "load_array.ETSGLOBAL::func"
//! ASM_INST        "Load 8"
//! ASM             arm64: /ldr w.., \[x.., w.., sxtw #2\]/, amd64: /movsxd .*movsxdl (.*)/

function main(): void {
    const byteArr: FixedArray<byte> = [0, 1, 2, 3, 4, 5, 6, -7]
    func(byteArr, 0, byteArr.length)
}

function func(arr: FixedArray<byte>, startIndex: int, endIndex: int): void {
    const cnts: FixedArray<int> = new FixedArray<int>(256)
    cnts[arr[7] + 128]++
}

/*
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

//! CHECKER      Forward gotos offsets in DISASM JIT
//! RUN          force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::.*'", entry: "gotos.ETSGLOBAL::main"
//! EVENT        /Compilation,gotos.ETSGLOBAL::main,.*,COMPILED/
//! EVENT        /Compilation,gotos.ETSGLOBAL::__noinline__while,.*,COMPILED/
//! ASM_METHOD   "gotos.ETSGLOBAL::__noinline__while"
//! ASM          arm64: "cbnz w16, #+0x30", x64: "jnz +0x39", arm: "bne 0x10000088"
//! ASM          arm64: "b.ge #+0x14", x64: "jnl +0x14", arm: "bge 0x10000068"
//! ASM          arm64: "b.eq #+0xc", x64: "jz +0x0C", arm: "beq 0x10000068"

function __noinline__while(a: int): int
{
    let i: int = 0;
    for(; i < 100; ++i) {
        if (i == a) {
            break;
        }
    }
    return i + 1;
}

function main(): int {
    arktest.assertEQ(__noinline__while(6), 7)
    return 0;
}

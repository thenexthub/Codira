/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! CHECKER       JIT: memmove unsupported tests
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='memmove_unsupported.TestUnsupported::.*'", entry: "memmove_unsupported.ETSGLOBAL::main"
//!
//! METHOD        "memmove_unsupported.TestUnsupported::noAlias_underIf"
//! PASS_AFTER    "LoopIdioms"
//! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_unsupported.TestUnsupported::patternMissmatch_boundscheck"
//! PASS_AFTER    "LoopIdioms"
//! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_unsupported.TestUnsupported::subIdx"
//! PASS_AFTER    "LoopIdioms"
//! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_unsupported.TestUnsupported::__noinline__noAlias_RefTypeCheck"
//! PASS_AFTER    "LoopIdioms"
//! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
//! INST          /RefTypeCheck/
//!
//! METHOD        "memmove_unsupported.TestUnsupported::noAlias_UpdateFirst_inLoop_ref"
//! PASS_AFTER    "LoopIdioms"
//! IN_BLOCK      "head, loop .*, depth 1, bc: 0x00000024"
//! INST_NOT      /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//! INST          /CallStatic.*__noinline__updateFirst/
//! INST_NEXT     /LoadArray/
//! INST_NEXT     /StoreArray/

//! CHECKER       AOT: memmove unsupported tests if there is not-inlined method (local or external)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='memmove_unsupported.TestUnsupported::.*' --compiler-inlining-blacklist='std.core.Array::<ctor>'", entry: "memmove_unsupported.ETSGLOBAL::main" # CC-OFF(G.FMT.02) special comment for testing
//!
//! METHOD        "memmove_unsupported.TestUnsupported::__noinline__noAlias_RefTypeCheck"
//! PASS_AFTER    "LoopIdioms"
//! INST_NOT      /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//! INST          /RefTypeCheck/
//!
//! METHOD        "memmove_unsupported.TestUnsupported::noAlias_UpdateFirst_inLoop_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST_NOT      /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//! INST          /CallStatic.*__noinline__updateFirst/
//! INST_NEXT     /LoadArray/
//! INST_NEXT     /StoreArray/

function check(arr1: Object, arr2: Object): int {
    if (arr1.toString() != arr2.toString()) {
        console.print("Expected " + arr2.toString() + " but got " + arr1.toString());
        return 1;
    }
    return 0;
}

let GLOBAL_OFFS: int = 1;

class TestUnsupported {
    static noAlias_underIf(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        return check(TestUnsupported.__noinline__noAlias_underIf(src, dst), [6, 7, 8, 4, 5]);
    }

    static __noinline__noAlias_underIf(src: FixedArray<int>, dst: FixedArray<int>): FixedArray<int> {
        if (src != dst) {
            for (let i: int = 0; i < src.length; i++) {
                dst[i] = src[i];
            }
        }
        return dst;
    }
    
    static patternMissmatch_boundscheck(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + GLOBAL_OFFS] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }
    
    static subIdx(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = src.length; i > 0; i--) {
            dst[dst.length - i] = src[src.length - i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static noAlias_RefTypeCheck(): int {
        let src: FixedArray<string> = ["1", "2", "3", "4"];
        return check(TestUnsupported.__noinline__noAlias_RefTypeCheck(src), src);
    }
    static __noinline__noAlias_RefTypeCheck(src: FixedArray<string>): FixedArray<string> {
        let dst: FixedArray<string> = new FixedArray<string>(4);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return dst;
    }
    
    static noAlias_UpdateFirst_inLoop_ref(): int {
        let src: FixedArray<string> = ["1", "2", "3", "4"];
        let dst: FixedArray<string> = new FixedArray<string>(4);
        for (let i: int = 0; i < src.length; i++) {
            TestUnsupported.__noinline__updateFirst(dst);
            dst[i] = src[i];
        }
        return check(dst, ["", "2", "3", "4"]);
    }
    static __noinline__updateFirst(a: FixedArray<string>) {
        a[0] = "";
    }
};

function main(): int {
    let res: int = 0
    res += TestUnsupported.noAlias_underIf()
    res += TestUnsupported.patternMissmatch_boundscheck()
    res += TestUnsupported.subIdx()
    res += TestUnsupported.noAlias_RefTypeCheck()
    res += TestUnsupported.noAlias_UpdateFirst_inLoop_ref()
    arktest.assertEQ(res, 0)
    return res
}

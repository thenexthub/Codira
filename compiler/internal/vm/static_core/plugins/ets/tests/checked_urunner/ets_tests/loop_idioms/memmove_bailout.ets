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

function check(arr1: Object, arr2: Object): int {
    if (arr1.toString() != arr2.toString()) {
        console.print("Expected " + arr2.toString() + " but got " + arr1.toString());
        return 1;
    }
    return 0;
}

class TestBailout {
    //! CHECKER       TestBailout1
    //! RUN           force_jit: true, options: "--compiler-regex='.*memmove_bailout.TestBailout::.*' --compiler-inlining-blacklist='memmove_bailout.TestBailout::mayAlias.*,memmove_bailout.TestBailout::patternMissmatch_offsets1,memmove_bailout.TestBailout::patternMissmatch_offsets2,memmove_bailout.TestBailout::refArray,memmove_bailout.TestBailout::differentTypes'", entry: "memmove_bailout.TestBailout::main"
    //!
    //! METHOD        "memmove_bailout.TestBailout::__noinline__mayAlias"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    //!
    //! METHOD        "memmove_bailout.TestBailout::__noinline__mayAlias_ref"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
    //!
    //! METHOD        "memmove_bailout.TestBailout::patternMissmatch_offsets1"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    //!
    //! METHOD        "memmove_bailout.TestBailout::patternMissmatch_offsets2"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    //!
    //! METHOD        "memmove_bailout.TestBailout::refArray"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/
    //!
    //! METHOD        "memmove_bailout.TestBailout::differentTypes"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerMemmoveUnchecked/

    //! CHECKER       AOT Test memmove bailout for references
    //! SKIP_IF       @architecture == "arm32"
    //! RUN_PAOC      options: "--compiler-regex='.*memmove_bailout.TestBailout::.*mayAlias.*'"
    //! METHOD        "memmove_bailout.TestBailout::__noinline__mayAlias_ref"
    //! PASS_AFTER    "LoopIdioms"
    //! INST_NOT      /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
    //!
    static mayAlias(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        return check(TestBailout.__noinline__mayAlias(src, dst), [6, 7, 8, 4, 5]);
    }

    static __noinline__mayAlias(src: FixedArray<int>, dst: FixedArray<int>): FixedArray<int> {
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return dst;
    }

    static mayAlias_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<object> = new FixedArray<object>(3);
        return check(TestBailout.__noinline__mayAlias_ref(src, dst), ["6", "7", "8"]);
    }

    static __noinline__mayAlias_ref(src: FixedArray<string>, dst: FixedArray<object>): FixedArray<object> {
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return dst;
    }

    static patternMissmatch_offsets1(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[src.length - 1 - i];
        }
        return check(dst, [8, 7, 6, 4, 5]);
    }

    static patternMissmatch_offsets2(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[dst.length - 1 - i] = src[i];
        }
        return check(dst, [1, 2, 8, 7, 6]);
    }

    static refArray(): int {
        let src = new Array<int>(6, 7, 8);
        let dst = new Array<int>(1, 2, 3, 4, 5);
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, [6, 7, 8, 4, 5]);
    }

    static differentTypes(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<long> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, [6, 7, 8, 4, 5]);
    }

    static main(): void {
        TestBailout.mayAlias();
        TestBailout.mayAlias_ref();
        TestBailout.patternMissmatch_offsets1();
        TestBailout.patternMissmatch_offsets2();
        TestBailout.refArray();
        TestBailout.differentTypes();
    }
}

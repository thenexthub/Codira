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

//! CHECKER       Test memmove
//! RUN           force_jit: true, options: "--compiler-regex='.*memmove_supported.TestSupported::test_patternMatch_offsets.*'", entry: "memmove_supported.ETSGLOBAL::main"
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets1"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets2"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets3"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets4"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets_sameNegOffset"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets_negOffsets"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets_negOffsets_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets_sameNegOffset_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets4_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets3_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets2_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets1_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/

//! CHECKER       AOT Test memmove for references
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*memmove_supported.TestSupported::test_patternMatch_offsets.*'"
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets_negOffsets_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets_sameNegOffset_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets4_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets3_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets2_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported.TestSupported::test_patternMatch_offsets1_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/

function check(arr1: Object, arr2: Object): int {
    if (arr1.toString() != arr2.toString()) {
        console.print("Expected " + arr2.toString() + " but got " + arr1.toString());
        return 1;
    }
    return 0;
}

let GLOBAL_OFFS: int = 1;

class TestSupported {
    static test_patternMatch_offsets1(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static test_patternMatch_offsets1_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, ["1", "2", "6", "7", "8"]);
    }

    static test_patternMatch_offsets2(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static test_patternMatch_offsets2_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, ["1", "2", "6", "7", "8"]);
    }

    static test_patternMatch_offsets3(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static test_patternMatch_offsets3_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        let offs = dst.length - src.length;
        for (let i: int = 0; i < src.length; i++) {
            dst[i + offs] = src[i];
        }
        return check(dst, ["1", "2", "6", "7", "8"]);
    }

    static test_patternMatch_offsets4(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, [1, 2, 6, 7, 8]);
    }

    static test_patternMatch_offsets4_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        let offs = dst.length - src.length;
        for (let i: int = offs; i < dst.length; i++) {
            dst[i] = src[i - offs];
        }
        return check(dst, ["1", "2", "6", "7", "8"]);
    }

    static test_patternMatch_offsets_sameNegOffset(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        for (let i: int = 1; i < src.length; i++) {
            dst[i - 1] = src[i - 1];
        }
        return check(dst, [6, 7, 3, 4, 5]);
    }

    static test_patternMatch_offsets_sameNegOffset_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        for (let i: int = 1; i < src.length; i++) {
            dst[i - 1] = src[i - 1];
        }
        return check(dst, ["6", "7", "3", "4", "5"]);
    }

    static test_patternMatch_offsets_negOffsets(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        for (let i: int = 2; i < src.length + 1; i++) {
            dst[i - 1] = src[i - 2];
        }
        return check(dst, [1, 6, 7, 4, 5]);
    }

    static test_patternMatch_offsets_negOffsets_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        for (let i: int = 2; i < src.length + 1; i++) {
            dst[i - 1] = src[i - 2];
        }
        return check(dst, ["1", "6", "7", "4", "5"]);
    }
};

function main(): int {
    let res: int = 0;
    res += TestSupported.test_patternMatch_offsets1();
    res += TestSupported.test_patternMatch_offsets1_ref();
    res += TestSupported.test_patternMatch_offsets2();
    res += TestSupported.test_patternMatch_offsets2_ref();
    res += TestSupported.test_patternMatch_offsets3();
    res += TestSupported.test_patternMatch_offsets3_ref();
    res += TestSupported.test_patternMatch_offsets4();
    res += TestSupported.test_patternMatch_offsets4_ref();
    res += TestSupported.test_patternMatch_offsets_sameNegOffset();
    res += TestSupported.test_patternMatch_offsets_sameNegOffset_ref();
    res += TestSupported.test_patternMatch_offsets_negOffsets();
    res += TestSupported.test_patternMatch_offsets_negOffsets_ref();
    arktest.assertEQ(res, 0);
    return res;
}

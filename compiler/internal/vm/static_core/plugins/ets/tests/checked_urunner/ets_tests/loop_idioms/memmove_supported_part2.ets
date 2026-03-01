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
//! RUN           force_jit: true, options: "--compiler-regex='.*memmove_supported_part2.TestSupported::.*test.*'", entry: "memmove_supported_part2.ETSGLOBAL::main"
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_noAlias_locals"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::__noinline__test_noAlias_localAndParam"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_backwardsClosed"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_backwardsOpen"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_forwardClosed"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerMemmoveUnchecked/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_forwardClosed_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_backwardsOpen_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_backwardsClosed_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_noAlias_locals_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_noAlias_updateFirstOnly_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/

//! CHECKER       AOT Test memmove for references
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*memmove_supported_part2.TestSupported::.*test.*'"
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_forwardClosed_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_backwardsOpen_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_backwardsClosed_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_noAlias_locals_ref"
//! PASS_AFTER    "LoopIdioms"
//! INST          /Intrinsic.*CompilerEtsMemmoveUnchecked_Ref/
//!
//! METHOD        "memmove_supported_part2.TestSupported::test_noAlias_updateFirstOnly_ref"
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
    static test_noAlias_locals(): int {
        let src: FixedArray<int> = [1, 2, 3, 4];
        let dst: FixedArray<int> = new FixedArray<int>(4);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, [1, 2, 3, 4]);
    }

    static test_noAlias_locals_ref(): int {
        let src: FixedArray<string> = ["1", "2", "3", "4"];
        let dst: FixedArray<string> = new FixedArray<string>(4);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, ["1", "2", "3", "4"]);
    }

    static test_noAlias_updateFirstOnly_ref(): int {
        let src: FixedArray<string> = ["1", "2", "3", "4"];
        let dst: FixedArray<string> = new FixedArray<string>(4);
        TestSupported.__noinline__updateFirst(dst);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return check(dst, ["1", "2", "3", "4"]);
    }

    static __noinline__updateFirst(a: FixedArray<string>) {
        a[0] = "";
    }

    static test_noAlias_localAndParam(): int {
        let src: FixedArray<int> = [1, 2, 3, 4];
        return check(TestSupported.__noinline__test_noAlias_localAndParam(src), src);
    }

    static __noinline__test_noAlias_localAndParam(src: FixedArray<int>): FixedArray<int> {
        let dst: FixedArray<int> = new FixedArray<int>(4);
        for (let i: int = 0; i < src.length; i++) {
            dst[i] = src[i];
        }
        return dst;
    }

    static test_backwardsClosed(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = src.length - 1; i >= 0; i--) {
            dst[i + 1] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }

    static test_backwardsClosed_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        for (let i: int = src.length - 1; i >= 0; i--) {
            dst[i + 1] = src[i];
        }
        return check(dst, ["1", "6", "7", "8", "5"]);
    }

    static test_backwardsOpen(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let x = GLOBAL_OFFS;
        for (let i: int = src.length - 1; i > 0; i--) {
            dst[i - 1] = src[i];
        }
        return check(dst, [7, 8, 3, 4, 5]);
    }

    static test_backwardsOpen_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        for (let i: int = src.length - 1; i > 0; i--) {
            dst[i - 1] = src[i];
        }
        return check(dst, ["7", "8", "3", "4", "5"]);
    }

    static test_forwardClosed(): int {
        let src: FixedArray<int> = [6, 7, 8];
        let dst: FixedArray<int> = [1, 2, 3, 4, 5];
        let offs = dst.length - src.length;
        let x = GLOBAL_OFFS;
        for (let i: int = 0; i <= src.length - 1; i++) {
            dst[i + 1] = src[i];
        }
        return check(dst, [1, 6, 7, 8, 5]);
    }

    static test_forwardClosed_ref(): int {
        let src: FixedArray<string> = ["6", "7", "8"];
        let dst: FixedArray<string> = ["1", "2", "3", "4", "5"];
        for (let i: int = 0; i <= src.length - 1; i++) {
            dst[i + 1] = src[i];
        }
        return check(dst, ["1", "6", "7", "8", "5"]);
    }
};

function main(): int {
    let res: int = 0;
    res += TestSupported.test_noAlias_locals();
    res += TestSupported.test_noAlias_locals_ref();
    res += TestSupported.test_noAlias_localAndParam();
    res += TestSupported.test_noAlias_updateFirstOnly_ref();
    res += TestSupported.test_backwardsClosed();
    res += TestSupported.test_backwardsClosed_ref();
    res += TestSupported.test_backwardsOpen();
    res += TestSupported.test_backwardsOpen_ref();
    res += TestSupported.test_forwardClosed();
    res += TestSupported.test_forwardClosed_ref();
    arktest.assertEQ(res, 0);
    return res;
}

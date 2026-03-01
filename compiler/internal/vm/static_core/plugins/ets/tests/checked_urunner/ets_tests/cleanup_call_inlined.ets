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

//! CHECKER       Remove redundant Call.Inlined and ReturnInlined
//! RUN           force_jit: true, options: "--compiler-regex='cleanup_call_inlined.ETSGLOBAL::test_.*'", entry: "cleanup_call_inlined.ETSGLOBAL::main"
//! METHOD        "cleanup_call_inlined.ETSGLOBAL::test_after_checks_elim"
//! PASS_AFTER    "Inline"
//! INST_COUNT    /Call.*Inlined/, 1
//! INST_COUNT    /ReturnInlined/, 1
//! PASS_AFTER    "ChecksElimination"
//! PASS_AFTER_NEXT "Cleanup" # skip cleanup in light mode
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT      /Inlined/
//! METHOD        "cleanup_call_inlined.ETSGLOBAL::test_after_escape"
//! PASS_AFTER    "Inline"
//! INST_COUNT    /Call.*Inlined/, 10
//! INST_COUNT    /ReturnInlined/, 10
//! PASS_AFTER    "EscapeAnalysis"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT      /Inlined/

function sum2(a: FixedArray<int>): int {
    return a[0] + a[1];
}

function test_after_checks_elim(): int {
    let a: FixedArray<int> = new FixedArray<int>(2);
    a[0] = 2;
    a[1] = 3;
    return sum2(a);
}

final class Counter {
    public add(value: int) : int {
        // NullCheck here can be removed only after ScalarReplacement and ChecksElimination,
        // after that all inlined call instructions can also be removed
        this.total += value;
        return this.total;
    }

    total: int = 0;
}

function add(c: WeakRef<Counter>, value: int) : int {
    return c.deref()!.add(value);
}

function test_after_escape(): int {
    let counter = new Counter();
    let ref = new WeakRef<Counter>(counter);
    add(ref, 4);
    add(ref, 5);
    return ref.deref()!.total;
}


function main(): int {
    if (test_after_checks_elim() != 5) {
        return 1;
    }
    if (test_after_escape() != 9) {
        return 2;
    }
    return 0;
}

/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER           AOT: EscapeAnalysis Phi aliasing regression
//! SKIP_IF           @architecture == "arm32"
//! RUN_PAOC          options: "--compiler-inline-external-methods-aot=true --compiler-regex='.*test.*'", entry: "escape_analysis_check_phi.ETSGLOBAL::main"
//! METHOD            "escape_analysis_check_phi.ETSGLOBAL::test"
//! PASS_BEFORE       "EscapeAnalysis"
//! INST_COUNT        "Phi", 2
//! INST_COUNT        /.*ref.*Phi.*/, 2
//! INST_COUNT        /.*f64.*Phi.*/, 0
//! INST_COUNT        /StoreObject.*SomeObject/, 12
//! INST_COUNT        /StoreObject.*HelperObject/, 2
//! PASS_AFTER        "EscapeAnalysis"
//! INST_COUNT        "Phi", 12
//! INST_COUNT        /.*f64.*Phi.*/, 4
//! INST_COUNT        /StoreObject.*SomeObject/, 0
//! INST_COUNT        /StoreObject.*HelperObject/, 2

//! CHECKER           JIT: EscapeAnalysis Phi aliasing regression
//! RUN               force_jit: true, options: "--compiler-regex='.*test.*'", entry: "escape_analysis_check_phi.ETSGLOBAL::main"
//! METHOD            "escape_analysis_check_phi.ETSGLOBAL::test"
//! PASS_BEFORE       "EscapeAnalysis"
//! INST_COUNT        "Phi", 2
//! INST_COUNT        /.*ref.*Phi.*/, 2
//! INST_COUNT        /.*f64.*Phi.*/, 0
//! INST_COUNT        /StoreObject.*SomeObject/, 12
//! INST_COUNT        /StoreObject.*HelperObject/, 2
//! PASS_AFTER        "EscapeAnalysis"
//! INST_COUNT        "Phi", 12
//! INST_COUNT        /.*f64.*Phi.*/, 4
//! INST_COUNT        /StoreObject.*SomeObject/, 0
//! INST_COUNT        /StoreObject.*HelperObject/, 2

class SomeObject {
    val: number;
    e: HelperObject;

    constructor(q: number) {
        this.val = q;
        this.e = new HelperObject(q);
    }

    constructor(q: number, e: HelperObject) {
        this.val = q;
        this.e = e;
    }
}

class HelperObject {
    e: number;
    constructor(e: number) {
        this.e = e;
    }
}

// Helps complicate the CFG and produce SaveState/phi nodes in some cases.
function tweak(x: number): number {
    return (x & 1) === 0 ? (x + 1) : (x - 1);
}

function process(params: SomeObject): number {
    let s: number = 0;
    for (let i = 0; i < 2; i++) {
        s += params.val;
    }
    if (s > 100) {
        s = tweak(s);
    } else {
        s = tweak(s + 2);
    }
    return s;
}

function __no_inline__do_some_job(j: SomeObject): void {
}

function test(HelperObject: HelperObject | undefined): void {
    const q1 = 120;
    const q2 = 450;

    let p1: SomeObject;
    let p2: SomeObject;

    if (HelperObject == undefined) {
        p1 = new SomeObject(q1);
        p2 = new SomeObject(q2);
    } else {
        p1 = new SomeObject(q1, HelperObject);
        p2 = new SomeObject(q2, HelperObject);
    }

    __no_inline__do_some_job(p1);
    __no_inline__do_some_job(p2);

    const r1 = process(p1);
    const r2 = process(p2);

    arktest.assertEQ(p1.val, q1);
    arktest.assertEQ(p2.val, q2);
    arktest.assertNE(p1.val, p2.val);

}

function main(): void {
    let e = new HelperObject(42);
    test(e);
}
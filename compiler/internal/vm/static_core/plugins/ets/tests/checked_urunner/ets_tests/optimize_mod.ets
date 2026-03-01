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

//! CHECKER           JIT - Positive : Check mod optimization.
//! SKIP_IF           @architecture == "arm32"
//! RUN               force_jit: true, options: "--compiler-regex='.*func[0-9]+.*' --compiler-inlining=false", entry: "optimize_mod.ETSGLOBAL::main"
//! METHOD            "optimize_mod.ETSGLOBAL::func1"
//! PASS_BEFORE       "Lowering"
//! INST_COUNT        /Mod /, 12
//! PASS_AFTER        "Lowering"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        /ModI/, 2
//! INST_COUNT        /Mod /, 0
//! INST_COUNT        /SelectImm GE/, 9
//!
//! METHOD            "optimize_mod.ETSGLOBAL::func2"
//! PASS_BEFORE       "Lowering"
//! INST_COUNT        /Mod /, 6
//! PASS_AFTER        "Lowering"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        /ModI/, 2
//! INST_COUNT        /Mod /, 0
//! INST_COUNT        /SelectImm GE/, 4

//! CHECKER           JIT - Negative : Check mod optimization.
//! SKIP_IF           @architecture == "arm32"
//! RUN               force_jit: true, options: "--compiler-regex='.*func[0-9]+.*' --compiler-inlining=false", entry: "optimize_mod.ETSGLOBAL::main"
//! METHOD            "optimize_mod.ETSGLOBAL::func3"
//! PASS_BEFORE       "Lowering"
//! INST_COUNT        /Mod /, 12
//! PASS_AFTER        "Lowering"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        /ModI/, 1
//! INST_COUNT        /Mod /, 6
//! INST_COUNT        /SelectImm GE/, 5
//!
//! METHOD            "optimize_mod.ETSGLOBAL::func4"
//! PASS_BEFORE       "Lowering"
//! INST_COUNT        /Mod /, 12
//! PASS_AFTER        "Lowering"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        /ModI/, 12
//! INST_COUNT        /Mod /, 0
//! INST_COUNT        /SelectImm GE/, 0
//!
//! METHOD            "optimize_mod.ETSGLOBAL::func5"
//! PASS_BEFORE       "Lowering"
//! INST_COUNT        /Mod /, 4
//! PASS_AFTER        "Lowering"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        /ModI/, 3
//! INST_COUNT        /Mod /, 0
//! INST_COUNT        /SelectImm GE/, 1
//!
//! METHOD            "optimize_mod.ETSGLOBAL::func6"
//! PASS_BEFORE       "Lowering"
//! INST_COUNT        /Mod /, 4
//! PASS_AFTER        "Lowering"
//! PASS_AFTER_NEXT   "Cleanup"
//! INST_COUNT        /ModI/, 4
//! INST_COUNT        /Mod /, 0
//! INST_COUNT        /SelectImm GE/, 0

class Obj {
    value : number = 0;
    constructor(value : number) {
        this.value = value;
    }
}

function func1() : Array<Obj> {
    let count : int = 3000;
    let res : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    let resources : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    for (let i = 0; i < 15; i++) {
        let random = Math.random() * (10) + 1;
        resources[i] = new Obj(random);
    }
    for (let i = 0; i < count; i++) {
        res[i % 5] = resources[i % 15];
    }
    let result : Array<Obj> = new Array<Obj>();
    for (let i = 0; i < 5; i++) {
        result.push(res[i]);
        result.push(resources[i + 10]);
    }
    arktest.assertEQ(result.length, 10);
    return result;
}

function func2(i : int) : Array<Obj> {
    let res : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    let resources : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    res[i % 5] = resources[i % 15];
    res[(i + 1) % 5] = resources[(i + 1) % 15];
    res[(i + 2) % 5] = resources[(i + 2) % 15];
    let result : Array<Obj> = new Array<Obj>();
    for (let j = 0; j < 3; j++) {
        result.push(res[j]);
        result.push(resources[j]);
    }
    arktest.assertEQ(result.length, 6);
    return result;
}

function func3(modVal : int) : Array<Obj> {
    let count : int = 3000;
    let res : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    let resources : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    for (let i = 0; i < 15; i++) {
        let random = Math.random() * (10) + 1;
        resources[i] = new Obj(random);
    }
    for (let i = 0; i < count; i++) {
        res[i % modVal] = resources[i % 15];
    }
    let result : Array<Obj> = new Array<Obj>();
    for (let i = 0; i < 5; i++) {
        result.push(res[i]);
        result.push(resources[i + 10]);
    }
    arktest.assertEQ(result.length, 10);
    return result;
}

function func4() : Array<Obj> {
    let count : int = 3000;
    let res : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    let resources : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    for (let i = 0; i < 15; i++) {
        let random = Math.random() * (10) + 1;
        resources[i] = new Obj(random);
    }
    for (let i = count - 1; i >= 0; i--) {
        res[i % 5] = resources[i % 15];
    }
    let result : Array<Obj> = new Array<Obj>();
    for (let i = 0; i < 5; i++) {
        result.push(res[i]);
        result.push(resources[i]);
    }
    arktest.assertEQ(result.length, 10);
    return result;
}

function func5(i : int) : Array<Obj> {
    let res : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    let resources : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    res[i % 5] = resources[i % 15];
    res[(i + 6) % 5] = resources[(i + 1) % 15];
    let result : Array<Obj> = new Array<Obj>();
    for (let j = 0; j < 2; j++) {
        result.push(res[j]);
        result.push(resources[j]);
    }
    arktest.assertEQ(result.length, 4);
    return result;
}

function func6(i : int) : Array<Obj> {
    let res : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    let resources : FixedArray<Obj> = [new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0),
                                       new Obj(0), new Obj(0), new Obj(0), new Obj(0), new Obj(0)];
    let result : Array<Obj> = new Array<Obj>();
    if (i > 1) {
        res[i % 5] = resources[i % 15];
        result.push(res[i]);
        result.push(resources[i]);
    } else {
        res[(i + 6) % 5] = resources[(i + 1) % 15];
        result.push(res[0]);
        result.push(resources[0]);
    }
    arktest.assertEQ(result.length, 2);
    return result;
}

function main() {

    // --------------------- Positive tests ---------------------- //

    // 1) normal case and some case which add constant is equal to mod constant
    let res : Array<Obj> = func1();
    arktest.assertEQ(res[0].value, res[1].value);
    arktest.assertEQ(res[2].value, res[3].value);
    arktest.assertEQ(res[4].value, res[5].value);
    arktest.assertEQ(res[6].value, res[7].value);
    arktest.assertEQ(res[8].value, res[9].value);

    // 2) mod inst is in a root loop
    res = func2(0);
    arktest.assertEQ(res[0].value, res[1].value);
    arktest.assertEQ(res[2].value, res[3].value);
    arktest.assertEQ(res[4].value, res[5].value);

    // --------------------- Negative tests ---------------------- //

    // 3) mod value is not const
    res = func3(5);
    arktest.assertEQ(res[0].value, res[1].value);
    arktest.assertEQ(res[2].value, res[3].value);
    arktest.assertEQ(res[4].value, res[5].value);
    arktest.assertEQ(res[6].value, res[7].value);
    arktest.assertEQ(res[8].value, res[9].value);

    // 4) mod inst's input0 is not an add inst
    res = func4();
    arktest.assertEQ(res[0].value, res[1].value);
    arktest.assertEQ(res[2].value, res[3].value);
    arktest.assertEQ(res[4].value, res[5].value);
    arktest.assertEQ(res[6].value, res[7].value);
    arktest.assertEQ(res[8].value, res[9].value);

    // 5) add constant is greater than mod constant
    res = func5(0);
    arktest.assertEQ(res[0].value, res[1].value);
    arktest.assertEQ(res[2].value, res[3].value);

    // 6) base mod is not dominate the other mods
    res = func6(2);
    arktest.assertEQ(res[0].value, res[1].value);
}

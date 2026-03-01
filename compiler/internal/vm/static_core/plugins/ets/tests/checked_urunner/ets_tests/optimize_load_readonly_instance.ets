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

class Base {
    readonly x: int
    readonly y: int

    constructor(x: int) {
        this.x = x * 2
        __noinline__doSomething(this)
        arktest.assertEQ(getBaseX(this), x * 2)
        this.x = x * 3
        __noinline__doSomething(this)
        arktest.assertEQ(getBaseX(this), x * 3)
        this.y = -10
    }
}

class NotReadonly {
    x: int

    constructor(x: int) {
        this.x = x * 2
        arktest.assertEQ(getNotReadonlyX(this), x * 2)
        __noinline__changeX(this)
        arktest.assertEQ(getNotReadonlyX(this), x * 2 + 1)
        this.x = x * 3
        __noinline__doSomething(this)
        arktest.assertEQ(getNotReadonlyX(this), x * 3)
    }
}

function __noinline__doSomething(o: Object) {
}

function __noinline__changeX(o: NotReadonly) {
    o.x++
}


function getBaseX(b: Base): int {
    return b.x
}

function getNotReadonlyX(b: NotReadonly): int {
    return b.x
}

//! CHECKER      LoadObject for readonly field, GVN applied or not, compare with interpreter
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX"

//! CHECKER      LoadObject for readonly field, GVN applied or not, JIT
//! RUN          force_jit: true, options: "--compiler-regex='(optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX|Base::<ctor>)' --compiler-inlining-blacklist='Base::<ctor>'", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX"
//! METHOD       "optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT      /LoadObject.*Base.x/
//! INST_COUNT    /LoadObject.*Base.y/, 1
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*Base.x/, 2
//! PASS_AFTER   "ChecksElimination"
//! PASS_AFTER_NEXT "GVN"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   /LoadObject.*Base.x/, 1
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*Base.x/, 1
//! INST_COUNT    /LoadObject.*Base.y/, 1
//! METHOD       "Base::<ctor>"
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*Base.x/, 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*Base.x/, 2

//! CHECKER      LoadObject for readonly field, GVN applied or not, AOT
//! RUN_PAOC     options: "--compiler-regex='(optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX|Base::<ctor>)' --compiler-inlining-blacklist='Base::<ctor>'", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX"
//! METHOD       "optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT      /LoadObject.*Base.x/
//! INST_COUNT    /LoadObject.*Base.y/, 1
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*Base.x/, 2
//! PASS_AFTER   "ChecksElimination"
//! PASS_AFTER_NEXT "GVN"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   /LoadObject.*Base.x/, 1
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*Base.x/, 1
//! INST_COUNT    /LoadObject.*Base.y/, 1
//! METHOD       "Base::<ctor>"
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*Base.x/, 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*Base.x/, 2
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetBaseX"

function checkGetBaseX(): int {
    let b = new Base(10)
    __noinline__doSomething(b)
    arktest.assertEQ(getBaseX(b), 30)
    for (let i = 0; i < 5; i++) {
        __noinline__doSomething(b)
        arktest.assertEQ(getBaseX(b), 30)
        arktest.assertEQ(b.y, -10)
    }
    return 0
}

//! CHECKER      LoadObject for NOT readonly field, GVN NOT applied, compare with interpreter
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX"

//! CHECKER      LoadObject for NOT readonly field, GVN NOT applied, JIT
//! RUN          force_jit: true, options: "--compiler-regex='(optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX|NotReadonly::<ctor>)' --compiler-inlining-blacklist='NotReadonly::<ctor>'", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX"
//! METHOD       "optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT      /LoadObject.*NotReadonly.x/
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 2
//! METHOD       "NotReadonly::<ctor>"
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 3
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 3
//! PASS_AFTER   "LSE"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 2

//! CHECKER      LoadObject for NOT readonly field, GVN NOT applied, AOT
//! RUN_PAOC     options: "--compiler-regex='(optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX|NotReadonly::<ctor>)' --compiler-inlining-blacklist='NotReadonly::<ctor>'", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX"
//! METHOD       "optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT      /LoadObject.*NotReadonly.x/
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 2
//! METHOD       "NotReadonly::<ctor>"
//! PASS_AFTER   "Inline"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 3
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadObject.*NotReadonly.x/, 3
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_instance.ETSGLOBAL::checkGetNotReadonlyX"

function checkGetNotReadonlyX(): int {
    let b = new NotReadonly(10)
    __noinline__doSomething(b)
    arktest.assertEQ(getNotReadonlyX(b), 30)
    for (let i = 0; i < 5; i++) {
        __noinline__doSomething(b)
        arktest.assertEQ(getNotReadonlyX(b), 30)
    }
    return 0
}

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
    static readonly x: int
    static readonly y: int

    static {
        Base.x = 2
        __noinline__doSomething()
        arktest.assertEQ(getBaseX(), 2)
        Base.x = 3
        __noinline__doSomething()
        arktest.assertEQ(getBaseX(), 3)
        Base.x = 4
        Base.y = 10
    }
}

class NotReadonly {
    static x: int

    static {
        NotReadonly.x = 2
        __noinline__changeX()
        arktest.assertEQ(getNotReadonlyX(), 3)
        __noinline__changeX()
        arktest.assertEQ(getNotReadonlyX(), 4)
        __noinline__doSomething()
        arktest.assertEQ(getNotReadonlyX(), 4)
    }
}

function __noinline__doSomething() {
}

function __noinline__changeX() {
    NotReadonly.x++
}

function getBaseX(): int {
    return Base.x
}

function getNotReadonlyX(): int {
    return NotReadonly.x
}

//! CHECKER      LoadStatic for readonly field, GVN not applied, compare with interpreter
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetBaseX"

//! CHECKER      LoadStatic for readonly field, GVN not applied, JIT
//! RUN          force_jit: true, options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::getBaseX|Base::<cctor>)'", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetBaseX"
//! EVENT        "Compilation,optimize_load_readonly_static.ETSGLOBAL::getBaseX"
//! EVENT_NEXT   "Compilation,Base::<cctor>"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::getBaseX"
//! PASS_AFTER   "Codegen"
//! INST         /LoadStatic.*Base.x/
//! METHOD       "Base::<cctor>"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "LoadStatic"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "LoadStatic", 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   "LoadStatic", 2

//! CHECKER      LoadStatic for readonly field, GVN not applied, AOT
//! RUN_PAOC     options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::getBaseX|Base::<cctor>)'"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::getBaseX"
//! PASS_AFTER   "Codegen"
//! INST         /LoadStatic.*Base.x/
//! METHOD       "Base::<cctor>"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "LoadStatic"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "LoadStatic", 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   "LoadStatic", 2
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetBaseX"

//! CHECKER      Replace LoadStatic for readonly field with constant in JIT when class is initialized
//! RUN          options: "--compiler-regex='ETSGLOBAL::getBaseX' --no-async-jit=true --compiler-hotness-threshold=3", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetBaseX"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::getBaseX"
//! PASS_AFTER   "IrBuilder"
//! INST         "LoadStatic"
//! PASS_AFTER   "Codegen"
//! INST_NOT     "LoadStatic"

function checkGetBaseX(): int {
    for (let i = 0; i < 5; i++) {
        arktest.assertEQ(getBaseX(), 4)
    }
    return 0
}

//! CHECKER      LoadStatic for NOT readonly field, GVN not applied, compare with interpreter
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetNotReadonlyX"

//! CHECKER      LoadStatic for NOT readonly field, GVN not applied, JIT
//! RUN          force_jit: true, options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::getNotReadonlyX|NotReadonly::<cctor>)'", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetNotReadonlyX"
//! EVENT        "Compilation,optimize_load_readonly_static.ETSGLOBAL::getNotReadonlyX"
//! EVENT_NEXT   "Compilation,NotReadonly::<cctor>"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::getNotReadonlyX"
//! PASS_AFTER   "Codegen"
//! INST         /LoadStatic.*NotReadonly.x/
//! METHOD       "NotReadonly::<cctor>"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "LoadStatic"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "LoadStatic", 3
//! PASS_AFTER   "Codegen"
//! INST_COUNT   "LoadStatic", 3

//! CHECKER      LoadStatic for NOT readonly field, GVN not applied, AOT
//! RUN_PAOC     options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::getNotReadonlyX|NotReadonly::<cctor>)'"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::getNotReadonlyX"
//! PASS_AFTER   "Codegen"
//! INST         /LoadStatic.*NotReadonly.x/
//! METHOD       "NotReadonly::<cctor>"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "LoadStatic"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "LoadStatic", 3
//! PASS_AFTER   "Codegen"
//! INST_COUNT   "LoadStatic", 3
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetNotReadonlyX"

//! CHECKER      Do NOT replace LoadStatic for NOT readonly field with constant in JIT when class is initialized
//! RUN          options: "--compiler-regex='ETSGLOBAL::getNotReadonlyX' --no-async-jit=true --compiler-hotness-threshold=3", entry: "optimize_load_readonly_static.ETSGLOBAL::checkGetNotReadonlyX"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::getNotReadonlyX"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "LoadStatic", 1
//! PASS_AFTER   "Codegen"
//! INST_COUNT   "LoadStatic", 1

function checkGetNotReadonlyX(): int {
    for (let i = 0; i < 5; i++) {
        arktest.assertEQ(getNotReadonlyX(), 4 + i)
        __noinline__changeX()
    }
    return 0
}

//! CHECKER      LoadStatic for readonly field, GVN applied, compare with interpreter
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkBaseX"

//! CHECKER      LoadStatic for readonly field, GVN applied, JIT
//! RUN          force_jit: true, options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::checkBaseX|Base::<cctor>)'", entry: "optimize_load_readonly_static.ETSGLOBAL::checkBaseX"
//! EVENT        "Compilation,optimize_load_readonly_static.ETSGLOBAL::checkBaseX"
//! EVENT_NEXT   "Compilation,Base::<cctor>"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::checkBaseX"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadStatic.*Base.x/, 2
//! INST_COUNT   /LoadStatic.*Base.y/, 1
//! PASS_AFTER   "GVN"
//! PASS_AFTER_NEXT  "Cleanup"
//! INST_COUNT   /LoadStatic.*Base.x/, 1
//! INST_COUNT   /LoadStatic.*Base.y/, 1
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadStatic.*Base.x/, 1
//! INST_COUNT   /LoadStatic.*Base.y/, 1

//! CHECKER      LoadStatic for readonly field, GVN applied, AOT
//! RUN_PAOC     options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::checkBaseX|Base::<cctor>)'"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::checkBaseX"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadStatic.*Base.x/, 2
//! INST_COUNT   /LoadStatic.*Base.y/, 1
//! PASS_AFTER   "GVN"
//! PASS_AFTER_NEXT  "Cleanup"
//! INST_COUNT   /LoadStatic.*Base.x/, 1
//! INST_COUNT   /LoadStatic.*Base.y/, 1
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadStatic.*Base.x/, 1
//! INST_COUNT   /LoadStatic.*Base.y/, 1
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkBaseX"

function checkBaseX(): int {
    arktest.assertEQ(Base.x, 4)
    __noinline__doSomething()
    arktest.assertEQ(Base.y, 10)
    arktest.assertEQ(Base.x, 4)
    return 0
}

//! CHECKER      GVN not applied for LoadStatic because field is not readonly, compare with interpreter
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX"

//! CHECKER      GVN not applied for LoadStatic because field is not readonly, JIT
//! RUN          force_jit: true, options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX|NotReadonly::<cctor>)'", entry: "optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX"
//! EVENT        "Compilation,optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX"
//! EVENT_NEXT   "Compilation,NotReadonly::<cctor>"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadStatic.*NotReadonly.x/, 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadStatic.*NotReadonly.x/, 2

//! CHECKER      GVN not applied for LoadStatic because field is not readonly, AOT
//! RUN_PAOC     options: "--compiler-regex='(optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX|NotReadonly::<cctor>)'"
//! METHOD       "optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadStatic.*NotReadonly.x/, 2
//! PASS_AFTER   "Codegen"
//! INST_COUNT   /LoadStatic.*NotReadonly.x/, 2
//! RUN          options: "--compiler-enable-jit=false", entry: "optimize_load_readonly_static.ETSGLOBAL::checkNotReadonlyX"

function checkNotReadonlyX(): int {
    arktest.assertEQ(NotReadonly.x, 4)
    __noinline__changeX()
    arktest.assertEQ(NotReadonly.x, 5)
    return 0
}
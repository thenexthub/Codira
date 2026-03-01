/**
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

import { Runtime } from "std/core"

//! CHECKER      Check type prop
//! RUN          force_jit: true, options: "--compiler-regex='.*test.*'", entry: "type_propagation.ETSGLOBAL::main"
//! METHOD       "type_propagation.ETSGLOBAL::test"
//! PASS_AFTER   "IrBuilder"

// NOTE(schernykh): find a way to enable
//! DISABLED_CHECKER      Check type prop (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*test.*'"
//! METHOD       "type_propagation.ETSGLOBAL::test"
//! PASS_AFTER   "IrBuilder"
//! RUN          options: "", entry: "type_propagation.ETSGLOBAL::main"

class A {
    // NOTE: class literal is not yet supported
    static klass = Class.current()
}

class B extends A {
    static klass = Class.current()
}

let boxClass = Class.of(new Box<Int>())
let objectClass = Class.of(new Object())

function testNewObjSimple() {
    let b = new B()
    let klass1 = Runtime.__getTypeInfo(b)
    arktest.assertEQ(klass1, B.klass)
}

function testNewObjPhi(x: boolean) {
    let a: Object | null = null
    if (x)
        a = new A()
    else
        a = new B()
    let klass1 = Runtime.__getTypeInfo(a)
    arktest.assertTrue(klass1 === undefined)
}

function testNewObjStoreLoad() {
    let box = new Box<A>()
    box.set(new B())
    let klass1 = Runtime.__getTypeInfo(box)
    let klass2 = Runtime.__getTypeInfo(box.get())
    box.set(new A())
    let klass3 = Runtime.__getTypeInfo(box.get())

    arktest.assertEQ(klass1, boxClass)
    arktest.assertEQ(klass2, B.klass)
    arktest.assertEQ(klass3, A.klass)
}

function __noinline__blackBox(x: Object): boolean {
    return true
}

function __noinline__blackBox() {
}


function testInvalidateSimple() {
    let box = new Box<A>()
    box.set(new B())
    let klass1 = Runtime.__getTypeInfo(box.get())
    __noinline__blackBox(box)
    let klass2 = Runtime.__getTypeInfo(box.get())
    arktest.assertEQ(klass1, B.klass)
    arktest.assertEQ(klass2, objectClass)
}

function testInvalidateInTriangle(x: boolean) {
    let box = new Box<A>()
    box.set(new B())
    if (x) {
        __noinline__blackBox(box)
    }
    let klass1 = Runtime.__getTypeInfo(box.get())
    arktest.assertEQ(klass1, objectClass)
}

function testInvalidateBeforeEdge(x: boolean) {
    let klass1: Class|undefined = undefined
    let klass2: Class|undefined = undefined
    let box = new Box<A>()
    box.set(new B())
    if (x) {
        klass1 = Runtime.__getTypeInfo(box.get())
    }
    __noinline__blackBox(box)
    if (x) {
        klass2 = Runtime.__getTypeInfo(box.get())
    }
    arktest.assertEQ(klass1, B.klass)
    arktest.assertEQ(klass2, objectClass)
}

let gObj: Object | null = null
class Holder {
    obj: Object | null = null

    static klass = Class.current()
}
let gHolder: Holder = new Holder()

function testEscapeToGObject(x: boolean) {
    let box = new Box<A>()
    box.set(new B())
    gObj = box
    if (x) {
        let klass1 = Runtime.__getTypeInfo(box.get())
        __noinline__blackBox()
        let klass2 = Runtime.__getTypeInfo(box.get())
        arktest.assertEQ(klass1, B.klass)
        arktest.assertEQ(klass2, objectClass)
    }
}

function testEscapeToGObjectField(x: boolean) {
    let box = new Box<A>()
    box.set(new B())
    __noinline__blackBox()
    gHolder.obj = box
    if (x) {
        let klass1 = Runtime.__getTypeInfo(box.get())
        __noinline__blackBox()
        let klass2 = Runtime.__getTypeInfo(box.get())
        arktest.assertEQ(klass1, B.klass)
        arktest.assertEQ(klass2, objectClass)
    }
}

function testLongEscapeChain() {
    let h1 = new Holder()
    let h2 = new Holder()
    let h3 = new Holder()
    let h4 = new Holder()

    let h5 = new Holder()
    let h6 = new Holder()
    h4.obj = new Holder()
    h6.obj = new Holder()

    gHolder.obj = h1

    __noinline__blackBox() // h1 escapes
    h3.obj = h4
    h5.obj = h6
    let klass1 = Runtime.__getTypeInfo(h4.obj) // Holder
    let klass2 = Runtime.__getTypeInfo(h6.obj) // Holder
    __noinline__blackBox()
    h2.obj = h3
    h1.obj = h2 // h2,h3,h4 escape
    let klass3 = Runtime.__getTypeInfo(h4.obj) // Holder
    __noinline__blackBox() // h2-h4 are invalidated
    let klass4 = Runtime.__getTypeInfo(h4.obj) // Object
    let klass5 = Runtime.__getTypeInfo(h6.obj) // Holder
    arktest.assertEQ(klass1, Holder.klass)
    arktest.assertEQ(klass2, Holder.klass)
    arktest.assertEQ(klass3, Holder.klass)
    // Note(#27093): enable after union processing implementation
    // arktest.assertEQ(klass4, ObjectClass)
    // Next line should be removed after union processing implementation
    arktest.assertEQ(klass4, undefined)
    arktest.assertEQ(klass5, Holder.klass)
}

function testEscapeChainInTriangle(x: boolean) {
    let h1 = new Holder()
    let h2 = new Holder()

    h1.obj = h1
    if (x) {
        let h3 = new Holder()
        h3.obj = h1
        h2.obj = h3
    }
    __noinline__blackBox(h2) // h3, h2, h1 escape and are invalidated
    // Note(#27093): enable after union processing implementation
    // arktest.assertEQ(Runtime.__getTypeInfo(h2.obj), objectClass)
    // arktest.assertEQ(Runtime.__getTypeInfo(h2.obj), objectClass)
    // Next two line should be removed after union processing implementation
    arktest.assertEQ(Runtime.__getTypeInfo(h2.obj), undefined)
    arktest.assertEQ(Runtime.__getTypeInfo(h2.obj), undefined)
}

function testLoopPhi() {
    let a = new A()
    __noinline__blackBox(a) // doesn't affect anything
    for (let i = 0; i < 11; i++) {
        arktest.assertEQ(Runtime.__getTypeInfo(a), A.klass)
        a = new A()
        __noinline__blackBox(a)
    }
}

function testLoopLoad() {
    let a = new A()
    gHolder.obj = a
    for (let i = 0; i < 5; i++) {
        // NOTE: doesn't work optimally, need fix
        // Note(#27093): enable after union processing implementation
        // arktest.assertEQ(Runtime.__getTypeInfo(gHolder.obj), objectClass)
        // Next line should be removed after union processing implementation
        arktest.assertEQ(Runtime.__getTypeInfo(gHolder.obj), undefined)
        gHolder.obj = new A()
    }
}

function testDiamondEscape(x: boolean) {
    let h1 = new Holder()
    let h2 = new Holder()
    h2.obj = new A()
    if (x) {
        gObj = h1
    } else {
        h1.obj = h2;
    }
    // we consider `holder` escaped although it cannot escape on each beanch of `if`
    __noinline__blackBox()
    let klass1 = Runtime.__getTypeInfo(h2.obj)
    // Note(#27093): enable after union processing implementation
    // arktest.assertEQ(klass1, objectClass)
    // Next line should be removed after union processing implementation
    arktest.assertEQ(klass1, undefined)
}

function main() {
    testNewObjSimple()
    testNewObjPhi(true)
    testNewObjStoreLoad()
    testInvalidateSimple()
    testInvalidateInTriangle(true)
    testInvalidateBeforeEdge(true)
    testEscapeToGObject(true)
    testEscapeToGObjectField(true)
    testLongEscapeChain()
    testEscapeChainInTriangle(true)
    testLoopPhi()
    testLoopLoad()
    testDiamondEscape(true)
}

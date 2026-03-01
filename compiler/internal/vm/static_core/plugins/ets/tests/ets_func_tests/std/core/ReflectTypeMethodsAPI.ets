/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

class A {
    public foo() {}
    public foo(a: string) {}
    public foo(a: number | char) {}

    public static stfoo() {}
    public static stfoo(a: int) {}

    private prfoo() {}
    private prfoo(a: string) {}
}

class B extends A {
    public foo(a: int) {}

    public static stfoo(a: double) {}

    private prfoo(a: float) {}
}

interface If1 {
    foo()
    foo(a: string)

    dffoo() {}
    dffoo(a: int | double) {}

    private prfoo() {}
    private prfoo(a: number) {}
}

interface If2 extends If1 {
    foo()
    foo(a: int)

    dffoo(a: string) {}
}

interface If3 extends If1, If2 {
    goo(a: int | char)

    dfgoo() {}

    private prgoo() {}
}

class C implements If1 {
    foo() {}
    foo(a: string) {}

    // dffoo() {} comes as default from If1
    dffoo(a: int | double) {} // override dffoo from If1
}

class D implements If1, If2 {
    foo() {}
    foo(a: int) {}
    foo(a: string) {}
}

class E {
    foo(a: int) {}
    foo(a: double) {}
}

interface If4 {
    boo(a: string)
    dfboo() {}
    dfboo(a: int) {}
}

interface If5 extends If4 {
    dfboo(a: int | number) {}
}

class F extends E implements If5 {
    foo(a: int) {} // override E.foo(a: int)
    boo(a: string) {}
    dfboo(a: int) {} // override If4.dfboo(a: int)
    // dfboo() {} comes from If4
    // dfboo(a: int | number) comes from If5
}

function compareTypes(lhs: FixedArray<Class>, rhs: FixedArray<Class>): boolean {
    if (lhs.length != rhs.length) {
        return false
    }
    for (let idx = 0; idx < lhs.length; ++idx) {
        if (lhs[idx] != rhs[idx]) {
            return false
        }
    }
    return true
}

function checkOwner(method: reflect.InstanceMethod, classType: Class) {
    arktest.assertNE(method.getOwner(), undefined)
    arktest.assertEQ(method.getOwner(), classType, `actual owner = '${method.getOwner()!.getName()}', expected '${classType.getName()}'`)
}

function checkOwner(method: reflect.StaticMethod, classType: Class) {
    arktest.assertNE(method.getOwner(), undefined)
    arktest.assertEQ(method.getOwner(), classType, `actual owner = '${method.getOwner()!.getName()}', expected '${classType.getName()}'`)
}

function testGetInstanceMethods(classType: Class, expectedMethodsArr: Array<reflect.InstanceMethod>) {
    let actualMethods = new Array<reflect.InstanceMethod>()
    for (let mtd of classType.getInstanceMethods()) {
        actualMethods.push(mtd)
    }

    arktest.assertEQ(actualMethods.length, expectedMethodsArr.length);

    for (let expectedMethod of expectedMethodsArr) {
        let methodName = expectedMethod.getName()
        let paramTypes = expectedMethod.getParametersTypes()

        let actualMethod = actualMethods.find((method: reflect.InstanceMethod) => {
            return method.getName() === methodName && compareTypes(method.getParametersTypes(), paramTypes)
        });
        arktest.assertNE(actualMethod, undefined, `Method ${methodName} with specified parameters should exist`);
        arktest.assertTrue(actualMethod!.equals(expectedMethod))
    }
}

function testGetStaticMethods(classType: Class, expectedMethodsArr: Array<reflect.StaticMethod>) {
    let actualMethods = new Array<reflect.StaticMethod>()
    for (let mtd of classType.getStaticMethods()) {
        actualMethods.push(mtd)
    }

    arktest.assertEQ(actualMethods.length, expectedMethodsArr.length);

    for (let expectedMethod of expectedMethodsArr) {
        let methodName = expectedMethod.getName()
        let paramTypes = expectedMethod.getParametersTypes()

        let actualMethod = actualMethods.find((method: reflect.StaticMethod) => {
            console.log(`current method = ${method.getName()}`)
            return method.getName() === methodName && compareTypes(method.getParametersTypes(), paramTypes)
        });
        arktest.assertNE(actualMethod, undefined, `Method ${methodName} with specified parameters should exist`);
        arktest.assertTrue(actualMethod!.equals(expectedMethod))
    }
}

function getErrorMsgAmbiguousMethod(name: string) {
    return `Found two methods with the same name '${name}'. Please specify signature.`
}

function singleClassSuite() {
    let aKlass = Class.from<A>()

    let errorMsg = getErrorMsgAmbiguousMethod("foo")
    const expectedErrors = (e: Error): boolean => {
        return e.message == errorMsg
    }
    arktest.expectThrow(()=>{ aKlass.getInstanceMethod("foo")}, expectedErrors)

    let foo1 = aKlass.getInstanceMethod("foo", [])
    let foo2 = aKlass.getInstanceMethod("foo", [ Class.from<string>() ])
    let foo3 = aKlass.getInstanceMethod("foo", [ Class.from<number | char>() ])
    checkOwner(foo1!, aKlass)
    checkOwner(foo2!, aKlass)
    checkOwner(foo3!, aKlass)
    testGetInstanceMethods(aKlass, [foo1!, foo2!, foo3!])

    errorMsg = getErrorMsgAmbiguousMethod("stfoo")
    arktest.expectThrow(()=>{ aKlass.getStaticMethod("stfoo")}, expectedErrors)
    let stfoo1 = aKlass.getStaticMethod("stfoo", [])
    let stfoo2 = aKlass.getStaticMethod("stfoo", [ Class.PRIMITIVE_INT ])
    checkOwner(stfoo1!, aKlass)
    checkOwner(stfoo2!, aKlass)
    testGetStaticMethods(aKlass, [stfoo1!, stfoo2!])

    let prfoo1 = aKlass.getInstanceMethod("prfoo", [])
    let prfoo2 = aKlass.getInstanceMethod("prfoo", [ Class.from<string>() ])
    arktest.assertEQ(prfoo1, undefined)
    arktest.assertEQ(prfoo2, undefined)
    arktest.assertEQ(aKlass.getInstanceMethod("prfoo"), undefined)
}

function classInheritSuite() {
    let bKlass = Class.from<B>()

    let foo1 = bKlass.getInstanceMethod("foo", [ Class.PRIMITIVE_INT ])
    checkOwner(foo1!, bKlass)
    arktest.assertTrue(foo1!.equals(bKlass.getInstanceMethod("foo")!))
    testGetInstanceMethods(bKlass, [ foo1! ])

    let stfoo1 = bKlass.getStaticMethod("stfoo", [ Class.PRIMITIVE_DOUBLE ])
    checkOwner(stfoo1!, bKlass)
    arktest.assertTrue(stfoo1!.equals(bKlass.getStaticMethod("stfoo")!))
    testGetStaticMethods(bKlass, [stfoo1!])

    let prfoo1 = bKlass.getInstanceMethod("prfoo", [ Class.PRIMITIVE_FLOAT ])
    arktest.assertEQ(prfoo1, undefined)
    arktest.assertEQ(bKlass.getInstanceMethod("prfoo"), undefined)
}

function classImplementSingleIfaceSuite() {
    let cKlass = Class.from<C>()

    let errorMsg = getErrorMsgAmbiguousMethod("foo")
    const expectedErrors = (e: Error): boolean => {
        return e.message == errorMsg
    }
    arktest.expectThrow(()=>{ cKlass.getInstanceMethod("foo")}, expectedErrors)

    let foo1 = cKlass.getInstanceMethod("foo", [])
    let foo2 = cKlass.getInstanceMethod("foo", [ Class.from<string>() ])
    let dffoo1 = cKlass.getInstanceMethod("dffoo", [])
    let dffoo2 = cKlass.getInstanceMethod("dffoo", [ Class.from<int | double>() ])
    checkOwner(foo1!, cKlass)
    checkOwner(foo2!, cKlass)
    checkOwner(dffoo1!, Class.from<If1>())
    checkOwner(dffoo2!, cKlass)
    testGetInstanceMethods(cKlass, [ foo1!, foo2!, dffoo1!, dffoo2! ])
}

function classInheritMultipleIfaceSuite() {
    let dKlass = Class.from<D>()

    let errorMsg = getErrorMsgAmbiguousMethod("foo")
    const expectedErrors = (e: Error): boolean => {
        return e.message == errorMsg
    }
    arktest.expectThrow(()=>{ dKlass.getInstanceMethod("foo")}, expectedErrors)
    errorMsg = getErrorMsgAmbiguousMethod("dffoo")
    arktest.expectThrow(()=>{ dKlass.getInstanceMethod("dffoo")}, expectedErrors)

    let foo1 = dKlass.getInstanceMethod("foo", [])
    let foo2 = dKlass.getInstanceMethod("foo", [ Class.PRIMITIVE_INT ])
    let foo3 = dKlass.getInstanceMethod("foo", [ Class.from<string>() ])
    let dffoo1 = dKlass.getInstanceMethod("dffoo", [])
    let dffoo2 = dKlass.getInstanceMethod("dffoo", [ Class.from<string>() ])
    let dffoo3 = dKlass.getInstanceMethod("dffoo", [ Class.from<int | double>() ])
    checkOwner(foo1!, dKlass)
    checkOwner(foo2!, dKlass)
    checkOwner(foo3!, dKlass)
    checkOwner(dffoo1!, Class.from<If1>())
    checkOwner(dffoo2!, Class.from<If2>())
    checkOwner(dffoo3!, Class.from<If1>())
    testGetInstanceMethods(dKlass, [ foo1!, foo2!, foo3!, dffoo1!, dffoo2!, dffoo3! ])
}

function classInheritClassAndImplementIfaceSuite() {
    let fKlass = Class.from<F>()

    let errorMsg = getErrorMsgAmbiguousMethod("dfboo")
    const expectedErrors = (e: Error): boolean => {
        return e.message == errorMsg
    }
    arktest.expectThrow(()=>{ fKlass.getInstanceMethod("dfboo")}, expectedErrors)

    let foo1 = fKlass.getInstanceMethod("foo", [ Class.PRIMITIVE_INT ])
    let boo1 = fKlass.getInstanceMethod("boo", [ Class.from<string>() ])
    let dfboo1 = fKlass.getInstanceMethod("dfboo", [ Class.PRIMITIVE_INT ])
    let dfboo2 = fKlass.getInstanceMethod("dfboo", [])
    let dfboo3 = fKlass.getInstanceMethod("dfboo", [ Class.from<int | number>()])
    checkOwner(foo1!, fKlass)
    checkOwner(boo1!, fKlass)
    checkOwner(dfboo1!, fKlass)
    checkOwner(dfboo2!, Class.from<If4>())
    checkOwner(dfboo3!, Class.from<If5>())
    testGetInstanceMethods(fKlass, [ foo1!, boo1!, dfboo1!, dfboo2!, dfboo3! ])
}

function singleInterfaceSuite() {
    let if1Klass = Class.from<If1>()

    let errorMsg = getErrorMsgAmbiguousMethod("foo")
    const expectedErrors = (e: Error): boolean => {
        return e.message == errorMsg
    }
    arktest.expectThrow(()=>{ if1Klass.getInstanceMethod("foo")}, expectedErrors)
    errorMsg = getErrorMsgAmbiguousMethod("dffoo")
    arktest.expectThrow(()=>{ if1Klass.getInstanceMethod("dffoo")}, expectedErrors)

    let foo1 = if1Klass.getInstanceMethod("foo", [])
    let foo2 = if1Klass.getInstanceMethod("foo", [ Class.from<string>() ])
    let dffoo1 = if1Klass.getInstanceMethod("dffoo", [])
    let dffoo2 = if1Klass.getInstanceMethod("dffoo", [ Class.from<int | double>() ])
    checkOwner(foo1!, if1Klass)
    checkOwner(foo2!, if1Klass)
    checkOwner(dffoo1!, if1Klass)
    checkOwner(dffoo2!, if1Klass)
    testGetInstanceMethods(if1Klass, [foo1!, foo2!, dffoo1!, dffoo2!])

    let prfoo1 = if1Klass.getInstanceMethod("prfoo", [])
    let prfoo2 = if1Klass.getInstanceMethod("prfoo", [ Class.from<number>() ])
    arktest.assertEQ(prfoo1, undefined)
    arktest.assertEQ(prfoo2, undefined)
    arktest.assertEQ(if1Klass.getInstanceMethod("prfoo"), undefined)
}

function interfaceExtendsSuite() {
    let if2Klass = Class.from<If2>()

    let errorMsg = getErrorMsgAmbiguousMethod("foo")
    const expectedErrors = (e: Error): boolean => {
        return e.message == errorMsg
    }
    arktest.expectThrow(()=>{ if2Klass.getInstanceMethod("foo")}, expectedErrors)

    let foo1 = if2Klass.getInstanceMethod("foo", [])
    let foo2 = if2Klass.getInstanceMethod("foo", [ Class.PRIMITIVE_INT ])
    let dffoo1 = if2Klass.getInstanceMethod("dffoo", [ Class.from<string>() ])
    checkOwner(foo1!, if2Klass)
    checkOwner(foo2!, if2Klass)
    checkOwner(dffoo1!, if2Klass)

    arktest.assertTrue(dffoo1!.equals(if2Klass.getInstanceMethod("dffoo")!))
    testGetInstanceMethods(if2Klass, [foo1!, foo2!, dffoo1!])
}

function interfaceExtendsMultipleSuite() {
    let if2Klass = Class.from<If3>()

    arktest.assertEQ(if2Klass.getInstanceMethod("foo"), undefined)
    arktest.assertEQ(if2Klass.getInstanceMethod("dffoo"), undefined)
    arktest.assertEQ(if2Klass.getInstanceMethod("prfoo"), undefined)

    let goo1 = if2Klass.getInstanceMethod("goo", [ Class.from<int | char>()])
    let dfgoo1 = if2Klass.getInstanceMethod("dfgoo", [])
    checkOwner(goo1!, if2Klass)
    checkOwner(dfgoo1!, if2Klass)
    arktest.assertEQ(if2Klass.getInstanceMethod("prgoo"), undefined)
    testGetInstanceMethods(if2Klass, [goo1!, dfgoo1!])
}

function main() {
    const suite = new arktest.ArkTestsuite('Type methods API tests')

    suite.addTest('Class', singleClassSuite)
    suite.addTest('Class inherit from other class', classInheritSuite)
    suite.addTest('Class implemented single interface', classImplementSingleIfaceSuite)
    suite.addTest('Class implemented multiple interfaces', classInheritMultipleIfaceSuite)
    suite.addTest('Class implemented interfaces and extended other class', classInheritClassAndImplementIfaceSuite)

    suite.addTest('Interface', singleInterfaceSuite)
    suite.addTest('Interface extending other interface', interfaceExtendsSuite)
    suite.addTest('Interface extending multiple interfaces', interfaceExtendsMultipleSuite)

    return suite.run()
}

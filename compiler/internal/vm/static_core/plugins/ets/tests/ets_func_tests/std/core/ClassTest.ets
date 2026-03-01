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

namespace NameSpace {
    export class NormalClass {
        x: int

    }

    export interface NormalInterface {

    }

    export class classImplingInterface implements NormalInterface {
        y: String
    }

    export enum NormalEnum {
        enumValue1,
        enumValue2
    }

    export function voidFunc(): void {}

    export function primFunc(): int {
        return 42
    }

    export function faFunc(): FixedArray<int> {
        let x: FixedArray<int> = [1, 2, 3]
        return x
    }

    export function objectFunc(): NormalClass {
        return new NormalClass()
    }

    export class funcContainer {
        public voidFunc(): void {}

        public primFunc(): int {
            return 42
        }

        public faFunc(): FixedArray<int> {
            let x: FixedArray<int> = [1, 2, 3]
            return x
        }

        public objectFunc(): NormalClass {
            return new NormalClass()
        }
    }

    interface HiddenInterface {

    }

    class HiddenClass implements HiddenInterface {

    }

    export function returnHiddenClassObject(): HiddenClass {
        return new HiddenClass()
    }

    enum G {
        enumValue1,
        enumValue2
    }

    export function returnHiddenEnumValue(): G {
        return G.enumValue1
    }

    export final class FinalClass {

    }

    final class HiddenFinalClass {

    }

    export function returnHiddenFinalClassObject(): HiddenFinalClass {
        return new HiddenFinalClass()
    }

    export abstract class AbstractClass {}
    export class ClassExtendingAbstract extends AbstractClass {

    }

    abstract class HiddenAbstractClass {}
    class HiddenClassExtendingAbstractClass extends HiddenAbstractClass {

    }

    export function returnHiddenClassExtendingAbstractClassObject(): HiddenClassExtendingAbstractClass {
        return new HiddenClassExtendingAbstractClass()
    }

    export interface InterfaceLiteral {
        i: int,
        str: String
    }

    export interface ObjectLiteral {
        i: int,
        str: String
    }

    export class Container {
        i: int
        str: String
        hiddenAbstractClass: HiddenClassExtendingAbstractClass
        hiddenFinalClass: HiddenFinalClass
        normalClass: NormalClass

        constructor() {
            this.str = "abc"
            this.hiddenAbstractClass = new HiddenClassExtendingAbstractClass()
            this.hiddenFinalClass = new HiddenFinalClass()
            this.normalClass = new NormalClass()
        }
    }
}

export class NormalClass {

}

export interface NormalInterface {

}

export class classImplingInterface implements NormalInterface {

}

export enum NormalEnum {
    enumValue1,
    enumValue2
}


export function voidFunc(): void {}

export function primFunc(): int {
    return 42
}

export function faFunc(): FixedArray<int> {
    let x: FixedArray<int> = [1, 2, 3]
    return x
}

export function objectFunc(): NormalClass {
    return new NormalClass()
}

export class funcContainer {
    public voidFunc(): void {}

    public primFunc(): int {
        return 42
    }

    public faFunc(): FixedArray<int> {
        let x: FixedArray<int> = [1, 2, 3]
        return x
    }

    public objectFunc(): NormalClass {
        return new NormalClass()
    }
}

export final class FinalClass {

}

export abstract class AbstractClass {}
export class ClassExtendingAbstract extends AbstractClass {

}


function checkClassIsNotEnumAndNotInterface() {
    for (let obj of [new NameSpace.classImplingInterface(), new NameSpace.NormalClass(), new classImplingInterface(), new NormalClass(), NameSpace.returnHiddenClassObject()]) {
        arktest.assertFalse(Class.of(obj).isEnum())
        arktest.assertFalse(Class.of(obj).isInterface())
    }
}

function checkEnumIsEnumAndNotInterface() {
    for (let obj of [NameSpace.NormalEnum.enumValue1, NormalEnum.enumValue1]) {
        arktest.assertTrue(Class.of(obj).isEnum())
        arktest.assertFalse(Class.of(obj).isInterface())
    }
    arktest.assertTrue(Class.of(NameSpace.returnHiddenEnumValue()).isEnum())
    arktest.assertFalse(Class.of(NameSpace.returnHiddenEnumValue()).isInterface())
}

function checkInterfaceIsInterfaceAndNotEnum() {
    let cls1 = Class.of(new NameSpace.classImplingInterface())
    let inf1 = cls1.getInterfaces()[0]
    let cls2 = Class.of(new classImplingInterface())
    let inf2 = cls2.getInterfaces()[0]
    let cls3 = Class.of(NameSpace.returnHiddenClassObject())
    let inf3 = cls3.getInterfaces()[0]
    for (let inf of [inf1, inf2, inf3]) {
        arktest.assertFalse(inf.isEnum())
        arktest.assertTrue(inf.isInterface())
    }
}

function checkFixedArray() {
    let varInt = 5
    let varDouble = 5.5
    let varPrimitiveArray: Array<int> = [1, 2, 3]
    let varObjectArray: Array<NormalClass> = [new NormalClass()]
    let varPrimitiveFixedArray: FixedArray<int> = [1, 2, 3]
    let varObjectFixedArray: FixedArray<NormalClass> = [new NormalClass()]
    let varObject = new NormalClass()
    let varString = "abc"
    arktest.assertFalse(Class.of(varInt).isFixedArray())
    arktest.assertFalse(Class.of(varDouble).isFixedArray())
    arktest.assertFalse(Class.of(varPrimitiveArray).isFixedArray())
    arktest.assertFalse(Class.of(varObjectArray).isFixedArray())
    arktest.assertTrue(Class.of(varPrimitiveFixedArray).isFixedArray())
    arktest.assertTrue(Class.of(varObjectFixedArray).isFixedArray())
    arktest.assertFalse(Class.of(varString).isFixedArray())
    arktest.assertFalse(Class.of(varObject).isFixedArray())
    arktest.assertFalse(Class.of(NormalEnum.enumValue1).isFixedArray())
    arktest.assertFalse(Class.of(new classImplingInterface()).getInterfaces()[0].isFixedArray())
    arktest.assertFalse(Class.of(faFunc).isFixedArray())
    arktest.assertFalse(Class.of(new funcContainer().faFunc).isFixedArray())
}

function checkFunctions() {
    let gc = new funcContainer()
    let nc = new NameSpace.funcContainer()
    let gList = [voidFunc, primFunc, faFunc, objectFunc]
    let gcList = [gc.voidFunc, gc.primFunc, gc.faFunc, gc.objectFunc]
    let nList = [NameSpace.voidFunc, NameSpace.primFunc, NameSpace.faFunc, NameSpace.objectFunc]
    let ncList = [nc.voidFunc, nc.primFunc, nc.faFunc, nc.objectFunc]
    for (let funcList of [gList, gcList, nList, ncList]) {
        for (let func of funcList) {
            let cls = Class.of(func)
            arktest.assertFalse(cls.isEnum())
            arktest.assertFalse(cls.isInterface())
            arktest.assertFalse(cls.isFixedArray())
        }
    }
}

class AA {}

class BB extends AA {}

class CC extends BB {}

interface I1 {}

interface I2 {}

interface I3 extends I2 {}

class C3 implements I3 {}

class Multi extends AA implements I1, I2 {}

function checkSubTypeOf() {
    let a = Class.of(new AA)
    let b = Class.of(new BB)
    let c = Class.of(new CC)

    arktest.assertTrue(c.isSubtypeOf(Class.of(new Object)), "C is subtype of object")

    arktest.assertTrue(a.isSubtypeOf(a), "AA is subtype of AA")
    arktest.assertTrue(b.isSubtypeOf(a), "BB is subtype of AA")
    arktest.assertTrue(c.isSubtypeOf(a), "CC is subtype of AA")
    arktest.assertTrue(c.isSubtypeOf(b), "CC is subtype of BB")
    arktest.assertFalse(a.isSubtypeOf(b), "AA is not subtype of BB")
    arktest.assertFalse(a.isSubtypeOf(c), "AA is not subtype of CC")
}

class MyUnion {
    public f1(): FixedArray<string> {
        return [""]
    }

    public f2(): FixedArray<object> {
        return [new Object]
    }

    public f3(): string | double {
        return 0.0
    }

    public f4(): string | double | Date {
        return new Date()
    }
}

class Gen<T> {
    public gen1<T>(x: T): T {
        return x
    }
    public gen2<T>(): T | undefined {
        return undefined
    }
}

function checkSubTypeOfUnion() {
    let u = Class.of(new MyUnion)
    let faStr = u.getInstanceMethod("f1")!.getReturnType()
    let faObj = u.getInstanceMethod("f2")!.getReturnType()
    arktest.assertTrue(faStr.isSubtypeOf(faObj))

    let f3 = u.getInstanceMethod("f3")!.getReturnType()
    let f4 = u.getInstanceMethod("f4")!.getReturnType()

    let obj = Class.of(new Object)
    arktest.assertTrue(f3.isSubtypeOf(obj))
    arktest.assertTrue(Class.of("").isSubtypeOf(f3))
    arktest.assertTrue(f3.isSubtypeOf(f4))

    let g = Class.of(new Gen<int>())
    let gen1 = g.getInstanceMethod("gen1")!.getReturnType()
    let gen2 = g.getInstanceMethod("gen2")!.getReturnType()

    arktest.assertEQ(gen1, gen2)
    arktest.assertEQ(gen1, obj)
}

function checkSubTypeOfMultipleInterfaces() {
    let a = Class.of(new AA)
    let b = Class.of(new BB)
    let m = Class.of(new Multi)
    let ifaces = m.getInterfaces()
    let i1 = ifaces[0]
    let i2 = ifaces[1]

    arktest.assertTrue(m.isSubtypeOf(a), "Multi is subtype of AA")
    arktest.assertTrue(m.isSubtypeOf(i1), "Multi is subtype of I1")
    arktest.assertTrue(m.isSubtypeOf(i2), "Multi is subtype of I2")
    arktest.assertFalse(m.isSubtypeOf(b), "Multi is not subtype of B")
}

function checkSubTypeOfInterfaces() {
    let m = Class.of(new Multi)
    let c3 = Class.of(new C3)
    let ifaces = m.getInterfaces()
    let i1 = ifaces[0]
    let i2 = ifaces[1]
    let i3 = c3.getInterfaces()[0]
    arktest.assertTrue(i3.isSubtypeOf(i2), "I3 extends I2")
    arktest.assertFalse(i2.isSubtypeOf(i3), "I2 doesn't extends I3")
}

function checkClassOfAny() {
    let clsObj: Class = Class.of(new Object)
    let clsNull: Class = Class.of(null)

    arktest.assertEQ(Class.ofAny(undefined), undefined)
    arktest.assertEQ(Class.ofAny(new Object), clsObj)
    arktest.assertEQ(Class.ofAny(null), clsNull)

    arktest.assertEQ(Class.ofAny(1), Class.ofAny(2))
    arktest.assertEQ(Class.ofAny(1.0f), Class.ofAny(2.0f))
    arktest.assertEQ(Class.ofAny(true), Class.ofAny(false))
    arktest.assertEQ(Class.ofAny(1.0), Class.ofAny(2.0))
    arktest.assertEQ(Class.ofAny("foo"), Class.ofAny("bar"))
    arktest.assertEQ(Class.ofAny(c'a'), Class.ofAny(c'b'))
    arktest.assertEQ(Class.ofAny([1,23]), Class.ofAny([1,2,3]))
}

function checkClassOfObject() {
    let clsObj: Class = Class.of(new Object)
    let clsNull: Class = Class.of(null)

    arktest.assertNE(clsObj, clsNull)
    arktest.assertEQ(clsObj.getName(), "std.core.Object")
    arktest.assertEQ(clsNull.getName(), "std.core.Null")

    arktest.assertEQ(Class.of(1), Class.of(2))
    arktest.assertEQ(Class.of(1.0f), Class.of(2.0f))
    arktest.assertEQ(Class.of(true), Class.of(false))
    arktest.assertEQ(Class.of(1.0), Class.of(2.0))
    arktest.assertEQ(Class.of("foo"), Class.of("bar"))
    arktest.assertEQ(Class.of(c'a'), Class.of(c'b'))
    arktest.assertEQ(Class.of([1,23]), Class.of([1,2,3]))
}

function checkFinal() {
    arktest.assertFalse(Class.of(new NormalClass()).isFinal())
    arktest.assertTrue(Class.of(new FinalClass()).isFinal())
    arktest.assertFalse(Class.of(new ClassExtendingAbstract()).getSuper()!.isFinal())
    arktest.assertFalse(Class.of(new NameSpace.NormalClass()).isFinal())
    arktest.assertTrue(Class.of(new NameSpace.FinalClass()).isFinal())
    arktest.assertFalse(Class.of(new NameSpace.ClassExtendingAbstract()).getSuper()!.isFinal())
    arktest.assertFalse(Class.of(NameSpace.returnHiddenClassObject()).isFinal())
    arktest.assertTrue(Class.of(NameSpace.returnHiddenFinalClassObject()).isFinal())
    arktest.assertFalse(Class.of(NameSpace.returnHiddenClassExtendingAbstractClassObject()).getSuper()!.isFinal())

    let simpleString = "abc"
    let utf16String = "\uCCCC"
    let subString = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
        .substring(0, 99)
        .substring(0, 98)
        .substring(0, 97)
        .substring(0, 96)
        .substring(0, 95)
        .substring(0, 94)
        .substring(0, 93)
        .substring(0, 92)
        .substring(0, 91)
        .substring(0, 90)
    let treeString = "9876543210"
        .concat("9876543210")
        .concat("9876543210")
        .concat("9876543210")
        .concat("9876543210")
        .concat("9876543210")
        .concat("9876543210")
        .concat("9876543210")
        .concat("9876543210")
        .concat("9876543210")

    arktest.assertTrue(Class.of(simpleString).isFinal())
    arktest.assertTrue(Class.of(utf16String).isFinal())
    arktest.assertTrue(Class.of(subString).isFinal())
    arktest.assertTrue(Class.of(treeString).isFinal())

    let il: NameSpace.InterfaceLiteral = {
        i: 5,
        str: "abc"
    }
    arktest.assertFalse(Class.of(il).isFinal())
    arktest.assertTrue(Class.of(il.i).isFinal())
    arktest.assertTrue(Class.of(il.str).isFinal())

    let ol: NameSpace.ObjectLiteral = {
        i: 5,
        str: "abc"
    }
    arktest.assertFalse(Class.of(ol).isFinal())
    arktest.assertTrue(Class.of(ol.i).isFinal())
    arktest.assertTrue(Class.of(ol.str).isFinal())

    let cont = new NameSpace.Container()
    arktest.assertTrue(Class.of(cont.i).isFinal())
    arktest.assertTrue(Class.of(cont.str).isFinal())
    arktest.assertFalse(Class.of(cont.hiddenAbstractClass).getSuper()!.isFinal())
    arktest.assertTrue(Class.of(cont.hiddenFinalClass).isFinal())
    arktest.assertFalse(Class.of(cont.normalClass).isFinal())
}

function checkAbstract() {
    arktest.assertFalse(Class.of(new NormalClass()).isAbstract())
    arktest.assertFalse(Class.of(new FinalClass()).isAbstract())
    arktest.assertTrue(Class.of(new ClassExtendingAbstract()).getSuper()!.isAbstract())
    arktest.assertFalse(Class.of(new NameSpace.NormalClass()).isAbstract())
    arktest.assertFalse(Class.of(new NameSpace.FinalClass()).isAbstract())
    arktest.assertTrue(Class.of(new NameSpace.ClassExtendingAbstract()).getSuper()!.isAbstract())
    arktest.assertFalse(Class.of(NameSpace.returnHiddenClassObject()).isAbstract())
    arktest.assertFalse(Class.of(NameSpace.returnHiddenFinalClassObject()).isAbstract())
    arktest.assertTrue(Class.of(NameSpace.returnHiddenClassExtendingAbstractClassObject()).getSuper()!.isAbstract())

    let il: NameSpace.InterfaceLiteral = {
        i: 5,
        str: "abc"
    }
    arktest.assertFalse(Class.of(il).isAbstract())
    arktest.assertFalse(Class.of(il.i).isAbstract())
    arktest.assertFalse(Class.of(il.str).isAbstract())

    let ol: NameSpace.ObjectLiteral = {
        i: 5,
        str: "abc"
    }
    arktest.assertFalse(Class.of(ol).isAbstract())
    arktest.assertFalse(Class.of(ol.i).isAbstract())
    arktest.assertFalse(Class.of(ol.str).isAbstract())

    let cont = new NameSpace.Container()
    arktest.assertFalse(Class.of(cont.i).isAbstract())
    arktest.assertFalse(Class.of(cont.str).isAbstract())
    arktest.assertTrue(Class.of(cont.hiddenAbstractClass).getSuper()!.isAbstract())
    arktest.assertFalse(Class.of(cont.hiddenFinalClass).isAbstract())
    arktest.assertFalse(Class.of(cont.normalClass).isAbstract())
}

function main(): int {
    let ts = new arktest.ArkTestsuite("Class API test")
    ts.addTest("checkClassIsNotEnumAndNotInterface", checkClassIsNotEnumAndNotInterface)
    ts.addTest("checkEnumIsEnumAndNotInterface", checkEnumIsEnumAndNotInterface)
    ts.addTest("checkInterfaceIsInterfaceAndNotEnum", checkInterfaceIsInterfaceAndNotEnum)
    ts.addTest("checkFixedArray", checkFixedArray)
    ts.addTest("checkFunctions", checkFunctions)
    ts.addTest('SubtypeOf', checkSubTypeOf)
    ts.addTest('Subtypeof multiple interfaces', checkSubTypeOfMultipleInterfaces)
    ts.addTest('Subtypeof interfaces', checkSubTypeOfInterfaces)
    ts.addTest("checkClassOfAny", checkClassOfAny)
    ts.addTest("checkClassOfObject", checkClassOfObject)
    ts.addTest("checkFinal", checkFinal)
    ts.addTest("checkAbstract", checkAbstract)
    ts.addTest("checkSubTypeOfUnion", checkSubTypeOfUnion)

    return ts.run();
}

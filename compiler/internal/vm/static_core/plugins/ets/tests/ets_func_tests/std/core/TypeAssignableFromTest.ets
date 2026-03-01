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

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name)
        return 0
    }
    console.println("FAILED: " + name)
    return 1
}

class A {}

class B extends A {}

class C extends A {}

interface I {
    foo() {}
}

class AI implements I {
    public foo() {}
}

function testClassType(): int {
    let at = Type.of(new A())
    let bt = Type.of(new B())
    let ct = Type.of(new C())
    return test(at.assignableFrom(bt), "A <- B") +
            test(at.assignableFrom(ct), "A <- C") +
            test(!bt.assignableFrom(ct), "B <\\- C") +
            test(!ct.assignableFrom(bt), "C <\\- B")
}

function testObjectType(): int {
    let ot = Type.of(new Object())
    let at = Type.of(new A())
    let it = (Type.of(new AI()) as ClassType).getInterface(0)
    let arrt = Type.of(new Double[3])
    let st = Type.of("str")
    let f : () => void = () : void => {
        return
    }
    let ft = Type.of(f)
    let intrt = Type.of(new Int(0))
    let intpt = Type.of(0)
    // TODO(kirill-mitkin): add enum and union type tests
    return test(ot.assignableFrom(at), "Object <- A") +
            test(ot.assignableFrom(it), "Object <- I") +
            test(ot.assignableFrom(arrt), "Object <- Double[]") +
            test(ot.assignableFrom(st), "Object <- String") +
            test(ot.assignableFrom(ft), "Object <- () : void") +
            test(ot.assignableFrom(intrt), "Object <- Int") +
            test(!ot.assignableFrom(intpt), "Object <\\- int")
}

interface I2 extends I {}

interface I3 extends I {}

class BI2 extends AI implements I2 {}

class BI3 extends AI implements I3 {}

class AI23 {
    i2: I2 = new BI2()
    i3: I3 = new BI3()
}

function testInterfaceType(): int {
    let ait = Type.of(new AI()) as ClassType
    let ai23t = Type.of(new AI23()) as ClassType
    let it = ait.getInterface(0)
    let i2t = ai23t.getFieldByName("i2").getType()
    let i3t = ai23t.getFieldByName("i3").getType()
    return test(it.assignableFrom(ait), "I <- AI") +
            test(it.assignableFrom(i2t), "I <- I2") +
            test(it.assignableFrom(i3t), "I <- I3") +
            test(!it.assignableFrom(ai23t), "I <\\- AI23") +
            test(!i2t.assignableFrom(i3t), "I2 <\\- I3") +
            test(!i3t.assignableFrom(i2t), "I3 <\\- I2")
}

function testArrayType(): int {
    let rat = Type.of(new double[1])
    let pat = Type.of(new Double[1])
    let arrat = Type.of(new A[1])
    let arrbt = Type.of(new B[1])
    let arrct = Type.of(new C[1])
    return test(!rat.assignableFrom(pat), "double[] <- Double[]") +
            test(!pat.assignableFrom(rat), "Double[] <- double[]") +
            test(arrat.assignableFrom(arrbt), "A[] <- B[]") +
            test(arrat.assignableFrom(arrct), "A[] <- C[]") +
            test(!arrbt.assignableFrom(arrct), "B[] <\\- C[]") +
            test(!arrct.assignableFrom(arrbt), "C[] <\\- B[]")
}

function testStringType(): int {
    let st = Type.of("literal")
    return test(st.assignableFrom(st), "String <- String")
}

class Base {}

class Derived extends Base {}

function testFunctionType(): int {
    let foo: () => Base = () : Base => { return new Base() }
    let foo1: (p: Base) => Base = (p: Base): Base => { return new Base()}
    let foo2: (p: Base) => Derived = (p: Base): Derived => { return new Derived() }
    let foo3: (p: Derived) => Base = (p: Derived): Base => { return new Base()}
    let foo4: (p: Derived) => Derived = (p: Derived): Derived => { return new Derived() }
    let ft = Type.of(foo)
    let f1t = Type.of(foo1)
    let f2t = Type.of(foo2)
    let f3t = Type.of(foo3)
    let f4t = Type.of(foo4)
    return test(f1t.assignableFrom(f2t), "(Base) -> Base <- (Base) -> Derived") +
            test(!f2t.assignableFrom(f1t), "(Base) -> Derived <\\- (Base) -> Base") +
            test(f3t.assignableFrom(f2t), "(Derived) -> Base <- (Base) -> Derived") +
            test(!f1t.assignableFrom(f4t), "(Base) -> Base <\\- (Derived) -> Derived") +
            test(!ft.assignableFrom(f1t), "() -> Base <\\- (Base) -> Base")
}

// TODO(kirill-mitkin): Add union and enum tests

function testNumericTypes(): int {
    let arr : Object[] = [new Char(), new Byte(), new Short(), new Int(), new Long(), new Float(), new Double()]
    let bt = Type.of(new Boolean())
    let arrTypes = new Array<Type>(arr.length)
    for (let i = 0; i < arr.length; i++) {
        arrTypes[i] = Type.of(arr[i])
    }
    let failures = 0
    for (let i = 0; i < arr.length; i++) {
        for (let j = 0; j < arr.length; j++) {
            test(arrTypes[i].assignableFrom(arrTypes[j]), arrTypes[i].toString() + " <- " + arrTypes[j].toString())
        }
        test(!bt.assignableFrom(arrTypes[i]), bt.toString() + " <\\- " + arrTypes[i].toString())
        test(!arrTypes[i].assignableFrom(bt), arrTypes[i].toString() + " <\\- " + bt.toString())
    }
    return failures
}

function testNullishTypes(): int {
    let ot = Type.of(new Object())
    let nt = NullType.REF
    let ut = UndefinedType.REF
    return test(!ot.assignableFrom(nt), "Object <\\- null") +
            test(!ot.assignableFrom(ut), "Object <\\- undefined")
}

function main(): int {
    let failures = 0
    failures += testClassType()
    failures += testObjectType()
    failures += testInterfaceType()
    failures += testArrayType()
    failures += testStringType()
    failures += testFunctionType()
    failures += testNumericTypes()
    failures += testNullishTypes()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}

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

function main() {
    const suite = new arktest.ArkTestsuite('Class.getName tests')
    suite.addTest('Class.getName FixedArray', testClassGetNameFixedArray)
    suite.addTest('Class.getName nested FixedArray', testClassGetNameNestedFixedArray)
    suite.addTest('Class.getName Objects', testClassGetNameObjects)
    suite.addTest('Class.getName Union', testClassGetNameUnion)
    suite.addTest('Class.getName with RuntimeLinker', testClassGetNameWithRuntimeLinker)
    suite.addTest('Class.getName Lambda', testClassGetNameLambda)
    suite.addTest('Class.getName primitives', testGetNamePrimitives)
    return suite.run()
}

class A {}

function getName(obj: Object): string {
    return Class.of(obj).getName()
}

function testClassGetNameFixedArray() {
    let fbool: FixedArray<boolean> = [true]
    let fbyte: FixedArray<byte> = [1]
    let fint: FixedArray<int> = [1]
    let fshort: FixedArray<short> = [1]
    let flong: FixedArray<long> = [1]
    let ffloat: FixedArray<float> = [1]
    let fdouble: FixedArray<double> = [1]
    let fnumber: FixedArray<number> = [1]
    let fchar: FixedArray<char> = [c'a']
    let fobject: FixedArray<Object> = [new Object]
    let ffunc: FixedArray<() => void> = [() => {}]
    let fa: FixedArray<A> = [new A]

    arktest.assertEQ(getName(fbool), "u1[]")
    arktest.assertEQ(getName(fbyte), "i8[]")
    arktest.assertEQ(getName(fint), "i32[]")
    arktest.assertEQ(getName(fshort), "i16[]")
    arktest.assertEQ(getName(flong), "i64[]")
    arktest.assertEQ(getName(ffloat), "f32[]")
    arktest.assertEQ(getName(fdouble), "f64[]")
    arktest.assertEQ(getName(fnumber), "f64[]")
    arktest.assertEQ(getName(fchar), "u16[]")
    arktest.assertEQ(getName(fobject), "std.core.Object[]")
    arktest.assertEQ(getName(ffunc), "std.core.Function0[]")
    arktest.assertEQ(getName(fa), "ClassGetNameTest.A[]")

    arktest.assertEQ(getBootRuntimeLinker().findLoadedClass(getName(fdouble))!.getName(), getName(fdouble))
    arktest.assertEQ(getBootRuntimeLinker().findLoadedClass(getName(fobject))!.getName(), getName(fobject))
}

function testClassGetNameNestedFixedArray() {
    let f: FixedArray<FixedArray<FixedArray<Array<int>>>> = [[[[123]]]]
    arktest.assertEQ(getName(f), "std.core.Array[][][]")
    arktest.assertEQ(getBootRuntimeLinker().findLoadedClass(getName(f))!.getName(), getName(f))
}

function testClassGetNameObjects() {
    let byteName = getName(new Byte(1))
    let shortName = getName(new Short(1))
    let intName = getName(new Int(1))
    let longName = getName(new Long(1))
    let floatName = getName(1.0f)
    let doubleName = getName(1.0)
    let charName = getName(c'a')
    let booleanName = getName(true)
    let bigintName = getName(1n)
    let stringName = getName("abcdefg")

    let className = getName(new A)

    arktest.assertEQ(byteName, "std.core.Byte")
    arktest.assertEQ(shortName, "std.core.Short")
    arktest.assertEQ(intName, "std.core.Int")
    arktest.assertEQ(longName, "std.core.Long")
    arktest.assertEQ(floatName, "std.core.Float")
    arktest.assertEQ(doubleName, "std.core.Double")
    arktest.assertEQ(charName, "std.core.Char")
    arktest.assertEQ(booleanName, "std.core.Boolean")
    arktest.assertEQ(bigintName, "std.core.BigInt")
    arktest.assertEQ(stringName, "std.core.String")

    arktest.assertEQ(getName(new Object), "std.core.Object")
    arktest.assertEQ(getName(new Map<int, int>), "std.core.Map")

    arktest.assertEQ(className, "ClassGetNameTest.A")

    let rl = getBootRuntimeLinker()
    arktest.assertEQ(rl.findLoadedClass(byteName)!.getName(), byteName)
    arktest.assertEQ(rl.findLoadedClass(shortName)!.getName(), shortName)
    arktest.assertEQ(rl.findLoadedClass(intName)!.getName(), intName)
    arktest.assertEQ(rl.findLoadedClass(longName)!.getName(), longName)
    arktest.assertEQ(rl.findLoadedClass(floatName)!.getName(), floatName)
    arktest.assertEQ(rl.findLoadedClass(doubleName)!.getName(), doubleName)
    arktest.assertEQ(rl.findLoadedClass(bigintName)!.getName(), bigintName)

    arktest.assertEQ(rl.loadClass(byteName, true)!.getName(), byteName)
    arktest.assertEQ(rl.loadClass(shortName, true)!.getName(), shortName)
    arktest.assertEQ(rl.loadClass(intName, true)!.getName(), intName)
    arktest.assertEQ(rl.loadClass(longName, true)!.getName(), longName)
    arktest.assertEQ(rl.loadClass(floatName, true)!.getName(), floatName)
    arktest.assertEQ(rl.loadClass(doubleName, true)!.getName(), doubleName)
    arktest.assertEQ(rl.loadClass(bigintName, true)!.getName(), bigintName)

    let abcLinker = new AbcRuntimeLinker(undefined, [])
    arktest.assertEQ(abcLinker.loadClass(byteName, true)!.getName(), byteName)
    arktest.assertEQ(abcLinker.loadClass(shortName, true)!.getName(), shortName)
    arktest.assertEQ(abcLinker.loadClass(intName, true)!.getName(), intName)
    arktest.assertEQ(abcLinker.loadClass(longName, true)!.getName(), longName)
    arktest.assertEQ(abcLinker.loadClass(floatName, true)!.getName(), floatName)
    arktest.assertEQ(abcLinker.loadClass(doubleName, true)!.getName(), doubleName)
    arktest.assertEQ(abcLinker.loadClass(bigintName, true)!.getName(), bigintName)
}

class LoLoLoLo {}

function testClassGetNameUnion() {
    let x: FixedArray<int | string | FixedArray<string | LoLoLoLo>> = [1, "2", [new LoLoLoLo]]
    arktest.assertEQ(getName(x), "{Ustd.core.Int,std.core.String,{UClassGetNameTest.LoLoLoLo,std.core.String}[]}[]")

    let y: FixedArray<FixedArray<FixedArray<FixedArray<byte | int | short>>>> = [[[[new Byte(1) , new Short(2), 3]]]]
    arktest.assertEQ(getName(y), "{Ustd.core.Byte,std.core.Int,std.core.Short}[][][][]")
}

function testClassGetNameWithRuntimeLinker() {
    let x: FixedArray<int | string | FixedArray<string | LoLoLoLo>> = [1, "2", [new LoLoLoLo]]
    let bootLinker = getBootRuntimeLinker()
    let xName = getName(x)
    arktest.assertEQ(xName, "{Ustd.core.Int,std.core.String,{UClassGetNameTest.LoLoLoLo,std.core.String}[]}[]")
    arktest.assertEQ(bootLinker.findLoadedClass(xName)!.getName(), xName)

    let y: FixedArray<FixedArray<FixedArray<FixedArray<byte | int | short>>>> = [[[[new Byte(1) , new Short(2), 3]]]]
    let yName = getName(y)
    arktest.assertEQ(yName, "{Ustd.core.Byte,std.core.Int,std.core.Short}[][][][]")
    arktest.assertEQ(bootLinker.findLoadedClass(yName)!.getName(), yName)
    arktest.assertEQ(bootLinker.loadClass(yName, true)!.getName(), yName)

    let abcLinker = new AbcRuntimeLinker(undefined, [])
    arktest.assertEQ(abcLinker.loadClass(yName, true)!.getName(), yName)
}

function foo() {}

function testClassGetNameLambda() {
    let l1 = () => {}
    let l2 = (x: int) => {}
    arktest.assertTrue(getName(l1).startsWith("ClassGetNameTest.%%lambda-lambda_invoke"))
    arktest.assertTrue(getName(l2).startsWith("ClassGetNameTest.%%lambda-lambda_invoke"))
    arktest.assertTrue(getName(foo).startsWith("ClassGetNameTest.%%lambda-lambda_invoke"))
}

function testGetNamePrimitives() {
    arktest.assertEQ(Class.PRIMITIVE_VOID.getName(), "void")
    arktest.assertEQ(Class.PRIMITIVE_INT.getName(), "i32")
    arktest.assertEQ(Class.PRIMITIVE_BOOLEAN.getName(), "u1")
    arktest.assertEQ(Class.PRIMITIVE_BYTE.getName(), "i8")
    arktest.assertEQ(Class.PRIMITIVE_CHAR.getName(), "u16")
    arktest.assertEQ(Class.PRIMITIVE_SHORT.getName(), "i16")
    arktest.assertEQ(Class.PRIMITIVE_INT.getName(), "i32")
    arktest.assertEQ(Class.PRIMITIVE_LONG.getName(), "i64")
    arktest.assertEQ(Class.PRIMITIVE_FLOAT.getName(), "f32")
    arktest.assertEQ(Class.PRIMITIVE_DOUBLE.getName(), "f64")
}

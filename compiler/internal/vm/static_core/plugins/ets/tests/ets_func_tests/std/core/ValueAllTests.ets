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

class Dummy {
    public a: int = 1
    public b: int = 2
}

function dummyFunc(x: int, y: int): int {
    return x + y
}

function returnVoid(): void {}


function testValueOfAndGetTypeAndGetData(): void {
    const inst = new Dummy()
    const val = reflect.Value.of(inst)

    const type = val.getType()
    arktest.assertTrue(type instanceof ClassType)
    const raw = val.getData()
    arktest.assertTrue(raw instanceof Dummy)
}

function testClassValueGetSetFieldByName(): void {
    const val = reflect.Value.of(new Dummy())
    arktest.assertTrue(val instanceof ClassValue)

    const cv = val as ClassValue
    cv.setFieldByName('a', reflect.Value.of(10))
    cv.setFieldByName('b', reflect.Value.of(20))

    arktest.assertEQ(10, cv.getFieldByName('a').getData())
    arktest.assertEQ(20, cv.getFieldByName('b').getData())
}

function testClassValueGetSetField(): void {
    const val = reflect.Value.of(new Dummy())
    arktest.assertTrue(val instanceof ClassValue)

    const cv = val as ClassValue
    const ct = cv.getType() as ClassType

    let aIndex = -1
    let bIndex = -1
    for (let i = 0; i < ct.getFieldsNum(); i++) {
        const f = ct.getField(i)
        if (f.getName() == 'a') aIndex = i
        if (f.getName() == 'b') bIndex = i
    }

    arktest.assertTrue(aIndex >= 0)
    arktest.assertTrue(bIndex >= 0)

    cv.setField(aIndex, reflect.Value.of(111 as int))
    cv.setField(bIndex, reflect.Value.of(222 as int))

    arktest.assertEQ(111, cv.getField(aIndex).getData())
    arktest.assertEQ(222, cv.getField(bIndex).getData())
}

function testArrayValueGetLength(): void {
    const val = reflect.Value.of([1, 2, 3])
    arktest.assertTrue(val instanceof ArrayValue)

    const av = val as ArrayValue
    arktest.assertEQ(3, av.getLength())
}

function testStringValueBasics(): void {
    const str = 'hello world'
    const val = reflect.Value.of(str)

    arktest.assertTrue(val instanceof StringValue)
    arktest.assertEQ('hello world', val.getData())
    const type = val.getType()
    arktest.assertTrue(type.equals(StringType.REF))
}

function testFunctionValueBasics(): void {
    const val = reflect.Value.of(dummyFunc)

    arktest.assertEQ(dummyFunc, val.getData())

    const type = val.getType()
    arktest.assertTrue(type instanceof FunctionType)
    const ft = type as FunctionType
    arktest.assertEQ(2 as long, ft.getParametersNum())
    arktest.assertTrue(ft.getResultType().equals(IntType.VAL))
}

function testUndefinedValueBasics(): void {
    const val = reflect.Value.of(undefined)

    arktest.assertTrue(val.getData() === undefined)

    const type = val.getType()
    arktest.assertTrue(type.equals(Type.of(undefined)))
}

function testNullValueBasics(): void {
    const val = reflect.Value.of(null)

    arktest.assertTrue(val.getData() === null)

    const type = val.getType()
    arktest.assertTrue(type.equals(Type.of(null)))
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Value & ClassValue tests')
    suite.addTest('Value.of / getType / getData', testValueOfAndGetTypeAndGetData)
    suite.addTest('ClassValue.getFieldByName / setFieldByName', testClassValueGetSetFieldByName)
    suite.addTest('ClassValue.getField / setField', testClassValueGetSetField)
    suite.addTest('ArrayValue.getLength', testArrayValueGetLength)
    suite.addTest('StringValue basics', testStringValueBasics)
    suite.addTest('FunctionValue basics', testFunctionValueBasics)
    suite.addTest('UndefinedValue basics', testUndefinedValueBasics)
    suite.addTest('NullValue basics', testNullValueBasics)
    return suite.run()
}

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

// A simple element class for reference‐type array tests
class Elem {
    public value: int;
    constructor() {
        this.value = 123;
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite('ArrayType API tests for SDK');

    suite.addTest('hasName on array', testArrayHasName);
    suite.addTest('getName on array', testArrayGetName);
    suite.addTest('getName on array (positive)', testArrayGetNamePositive);
    suite.addTest('getElementType for primitive', testElementTypePrimitive);
    suite.addTest('getElementType for reference', testElementTypeReference);
    suite.addTest('make primitive array', testMakePrimitiveArray);
    suite.addTest('make zero-length array', testMakeZeroLengthArray);

    return suite.run();
}

function testArrayHasName() {
    const at = Type.of(new FixedArray<int>(0)) as ArrayType;
    arktest.assertFalse(
        at.hasName(),
        'ArrayType.hasName() should be false'
    );
}

function testArrayHasNamePositive() {
    const at = Type.of(new FixedArray<Elem>(0)) as ArrayType;
    arktest.assertTrue(
        at.hasName(),
        'ArrayType.hasName() should be true for named reference types'
    );
}

function testArrayGetNamePositive() {
    const at = Type.of(new FixedArray<Elem>(0)) as ArrayType;
    arktest.assertEQ(
        'ArrayTypeAllTests.Elem',
        at.getElementType().getName(),
        `Expected name 'ArrayTypeAllTests.Elem', got '${at.getElementType().getName()}'`
    );
}

function testArrayGetName() {
    const at = Type.of(new FixedArray<float>(0)) as ArrayType;
    arktest.assertEQ(
        '',
        at.getName(),
        'ArrayType.getName() should return empty string'
    );
}

function testElementTypePrimitive() {
    const at = Type.of(new FixedArray<boolean>(0)) as ArrayType;
    const et = at.getElementType();
    arktest.assertEQ(
        'boolean',
        et.getLiteral(),
        `Expected element literal 'boolean', got '${et.getLiteral()}'`
    );
}

function testElementTypeReference() {
    const at = Type.of(new FixedArray<Elem>(0)) as ArrayType;
    const et = at.getElementType();
    arktest.assertTrue(
        et.hasName(),
        'Reference element type should have name'
    );
    arktest.assertEQ(
        'ArrayTypeAllTests.Elem',
        et.getName(),
        `Expected element name 'ArrayTypeAllTests.Elem', got '${et.getName()}'`
    )
}

function testMakePrimitiveArray() {
    const at = Type.of(new FixedArray<int>(0)) as ArrayType;
    const arr = at.make(5) as FixedArray<int>
    arktest.assertEQ(
        5 as long,
        arr.length,
        `Expected length 5, got ${arr.length}`
    );
    arktest.assertEQ(
        0 as int,
        arr[0],
        `Expected default element 0, got ${arr[0]}`
    );
}

function testMakeZeroLengthArray() {
    const at = Type.of(new FixedArray<string>(0)) as ArrayType;
    const arr = at.make(0) as FixedArray<string>;
    arktest.assertEQ(
        0 as long,
        arr.length,
        `Expected zero-length array, got length ${arr.length}`
    );
}

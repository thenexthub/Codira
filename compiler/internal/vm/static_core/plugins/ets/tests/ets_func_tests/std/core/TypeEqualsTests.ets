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

function main(): int {
    const suite = new arktest.ArkTestsuite('Type.equals API tests for SDK');
    suite.addTest('Primitive equals', testPrimitiveEquals);
    suite.addTest('Primitive vs Boxed equals', testPrimitiveBoxedEquals);
    suite.addTest('Boolean different values equals', testBooleanDifferentValuesEquals);
    suite.addTest('String equals', testStringEquals);
    suite.addTest('Class equals', testClassEquals);
    suite.addTest('Array equals', testArrayEquals);
    suite.addTest('Different types not equals', testDifferentTypesNotEquals);
    suite.addTest('Boolean equals', tesBooleanTypeEquals);
    return suite.run();
}

function testPrimitiveEquals() {
    arktest.assertTrue(
        Type.of(5 as int).equals(Type.of(5 as int)),
        'int should equal int'
    );
}

function testPrimitiveBoxedEquals() {
    const primType = Type.of(5 as int);
    const boxedType = Type.of(new Int(5));
    arktest.assertTrue(
        primType.equals(boxedType),
        'primitive int should equal boxed Int'
    );
}

function testBooleanDifferentValuesEquals() {
    const t = Type.of(true as boolean);
    const f = Type.of(false as boolean);
    arktest.assertTrue(
        t.equals(f),
        'boolean should equal boolean'
    );
}

function testStringEquals() {
    const s1 = Type.of('a');
    const s2 = Type.of('b');
    arktest.assertTrue(
        s1.equals(s2),
        'string should equal string'
    );
}

class A { }

function testClassEquals() {
    const obj1 = new A();
    const obj2 = new A();
    const t1 = Type.of(obj1);
    const t2 = Type.of(obj2);
    arktest.assertTrue(
        t1.equals(t2),
        'A should equal A'
    );
}

function testArrayEquals() {
    const arr1: int[] = [1, 2];
    const arr2: int[] = [3, 4];
    const t1 = Type.of(arr1);
    const t2 = Type.of(arr2);
    arktest.assertTrue(
        t1.equals(t2),
        'int[] should equal int[]'
    );
}

function testDifferentTypesNotEquals() {
    const tInt = Type.of(1 as int);
    const tBool = Type.of(true as boolean);
    arktest.assertFalse(
        tInt.equals(tBool),
        'int should not equal boolean'
    );
}

function tesBooleanTypeEquals() {
    const val = BooleanType.VAL;
    const ref = BooleanType.REF;

    arktest.assertTrue(val.equals(BooleanType.VAL));
    arktest.assertTrue(ref.equals(BooleanType.REF));
    arktest.assertTrue(val.equals(ref));
    arktest.assertTrue(ref.equals(val));
}

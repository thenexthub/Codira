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

class Foo { }
class A { }
class B extends A { }

function main(): int {
    const suite = new arktest.ArkTestsuite('Type.assignableFrom API tests for SDK');
    suite.addTest('Numeric assignableFrom', testNumericAssignable);
    suite.addTest('Boolean assignableFrom', testBooleanAssignable);
    suite.addTest('Class inheritance assignableFrom', testClassInheritanceAssignable);
    suite.addTest('Array assignableFrom', testArrayAssignable);
    suite.addTest('ObjectType assignableFrom', testObjectAssignable);
    return suite.run();
}

function testNumericAssignable() {
    const byteType = Type.of(1 as byte);
    const shortType = Type.of(1 as short);
    const intType = Type.of(1 as int);
    const longType = Type.of(1 as long);
    const floatType = Type.of(1.0 as float);
    const doubleType = Type.of(1.0 as double);

    arktest.assertTrue(shortType.assignableFrom(byteType), 'short <- byte');
    arktest.assertTrue(intType.assignableFrom(shortType), 'int <- short');
    arktest.assertTrue(longType.assignableFrom(intType), 'long <- int');
    arktest.assertTrue(floatType.assignableFrom(longType), 'float <- long');
    arktest.assertTrue(doubleType.assignableFrom(floatType), 'double <- float');
}

function testBooleanAssignable() {
    const boolType = Type.of(true as boolean);
    arktest.assertTrue(
        boolType.assignableFrom(Type.of(false as boolean)),
        'boolean <- boolean should be true'
    );
    arktest.assertFalse(
        boolType.assignableFrom(Type.of(1 as int)),
        'boolean <- int should be false'
    );
}

function testClassInheritanceAssignable() {
    const aType = Type.of(new A());
    const bType = Type.of(new B());
    arktest.assertTrue(
        aType.assignableFrom(bType),
        'A <- B should be true'
    );
    arktest.assertFalse(
        bType.assignableFrom(aType),
        'B <- A should be false'
    );
}

function testArrayAssignable() {
    const arrInt1: int[] = [1, 2];
    const arrInt2: int[] = [3, 4];
    arktest.assertTrue(
        Type.of(arrInt1).assignableFrom(Type.of(arrInt2)),
        'std.core.Array <- std.core.Array should be true'
    );
}

function testObjectAssignable() {
    const objType = Type.of({} as Object);
    const fooType = Type.of(new Foo());
    arktest.assertTrue(
        objType.assignableFrom(fooType),
        'Object <- Foo should be true'
    );
    arktest.assertFalse(
        fooType.assignableFrom(objType),
        'Foo <- Object should be false'
    );
}

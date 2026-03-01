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
    const suite = new arktest.ArkTestsuite('Type.of API tests for SDK');
    suite.addTest('Test Type.of boolean (primitive)', testTypeOfBooleanPrimitive);
    suite.addTest('Test Type.of Boolean (boxed)', testTypeOfBooleanBoxed);
    suite.addTest('Test Type.of char (primitive)', testTypeOfCharPrimitive);
    suite.addTest('Test Type.of Char (boxed)', testTypeOfCharBoxed);
    suite.addTest('Test Type.of int (primitive)', testTypeOfIntPrimitive);
    suite.addTest('Test Type.of Int (boxed)', testTypeOfIntBoxed);
    suite.addTest('Test Type.of byte (primitive)', testTypeOfBytePrimitive);
    suite.addTest('Test Type.of short (primitive)', testTypeOfShortPrimitive);
    suite.addTest('Test Type.of long (primitive)', testTypeOfLongPrimitive);
    suite.addTest('Test Type.of float (primitive)', testTypeOfFloatPrimitive);
    suite.addTest('Test Type.of double (primitive)', testTypeOfDoublePrimitive);
    suite.addTest('Test Type.of string', testTypeOfString);
    suite.addTest('Test Type.of int[]', testTypeOfIntArray);
    suite.addTest('Test Type.of string[]', testTypeOfStringArray);
    suite.addTest('Test Type.of object literal', testTypeOfObjectLiteral);
    suite.addTest('Test Type.of custom class', testTypeOfCustomClass);
    suite.addTest('Test Type.of null and undefined', testTypeOfNullUndefined);
    return suite.run();
}

function testTypeOfBooleanPrimitive() {
    const primType = Type.of(true as boolean);
    arktest.assertEQ(
        'boolean',
        primType.toString(),
        `Expected 'boolean', got '${primType.toString()}'`
    );
}

function testTypeOfBooleanBoxed() {
    const boxed = new Boolean(true);
    const type = Type.of(boxed);
    arktest.assertEQ(
        'boolean',
        type.toString(),
        `Expected 'boolean', got '${type.toString()}'`
    );
}

function testTypeOfCharPrimitive() {
    let c: char = c'x';
    const primType = Type.of(c);
    arktest.assertEQ(
        'char',
        primType.toString(),
        `Expected 'char', got '${primType.toString()}'`
    );
}

function testTypeOfCharBoxed() {
    const boxed = new Char(c'y');
    const type = Type.of(boxed);
    arktest.assertEQ(
        'char',
        type.toString(),
        `Expected 'char', got '${type.toString()}`
    );
}

function testTypeOfIntPrimitive() {
    let i: int = 42;
    const primType = Type.of(i);
    arktest.assertEQ(
        'int',
        primType.toString(),
        `Expected 'int', got '${primType.toString()}'`
    );
}

function testTypeOfIntBoxed() {
    const boxed = new Int(42);
    const type = Type.of(boxed);
    arktest.assertEQ(
        'int',
        type.toString(),
        `Expected 'int', got '${type.toString()}'`
    );
}

function testTypeOfBytePrimitive() {
    let b: byte = 1;
    const primType = Type.of(b);
    arktest.assertEQ(
        'byte',
        primType.toString(),
        `Expected 'byte', got '${primType.toString()}'`
    );
}

function testTypeOfShortPrimitive() {
    let s: short = 2;
    const primType = Type.of(s);
    arktest.assertEQ(
        'short',
        primType.toString(),
        `Expected 'short', got '${primType.toString()}'`
    );
}

function testTypeOfLongPrimitive() {
    let l: long = 3;
    const primType = Type.of(l);
    arktest.assertEQ(
        'long',
        primType.toString(),
        `Expected 'long', got '${primType.toString()}'`
    );
}

function testTypeOfFloatPrimitive() {
    let f: float = 1.5f;
    const primType = Type.of(f);
    arktest.assertEQ(
        'float',
        primType.toString(),
        `Expected "float", got "${primType.toString()}"`
    );
}

function testTypeOfDoublePrimitive() {
    let d: double = 2.5;
    const primType = Type.of(d);
    arktest.assertEQ(
        'double',
        primType.toString(),
        `Expected 'double', got '${primType.toString()}'`
    );
}

function testTypeOfString() {
    const type = Type.of('hello');
    arktest.assertEQ(
        'string',
        type.toString(),
        `Expected 'string', got '${type.toString()}'`
    );
}

function testTypeOfIntArray() {
    let arr: int[] = [1, 2];
    const type = Type.of(arr);
    arktest.assertEQ(
        'std.core.Array',
        type.toString(),
        `Expected 'std.core.Array', got '${type.toString()}'`
    );
}

function testTypeOfStringArray() {
    let arr: string[] = ['a', 'b'];
    const type = Type.of(arr);
    arktest.assertEQ(
        'std.core.Array',
        type.toString(),
        `Expected 'std.core.Array', got '${type.toString()}'`
    );
}

function testTypeOfObjectLiteral() {
    let obj: Object = {};
    const type = Type.of(obj);
    arktest.assertEQ(
        'std.core.Object',
        type.toString(),
        `Expected 'std.core.Object', got '${type.toString()}'`
    );
}

class Foo { }

function testTypeOfCustomClass() {
    const foo = new Foo();
    const type = Type.of(foo);
    arktest.assertEQ(
        'TypeOfTests.Foo',
        type.toString(),
        `Expected 'TypeOfTests.Foo', got '${type.toString()}'`
    );
}

function testTypeOfNullUndefined() {
    const nullType = Type.of(null);
    arktest.assertEQ(
        'null',
        nullType.toString(),
        `Expected 'null', got '${nullType.toString()}'`
    );

    const undefType = Type.of(undefined);
    arktest.assertEQ(
        'undefined',
        undefType.toString(),
        `Expected 'undefined', got '${undefType.toString()}'`
    );
}

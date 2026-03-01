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
    const suite = new arktest.ArkTestsuite('Type.toString API tests for SDK');
    suite.addTest('Primitive types toString', testPrimitiveToString);
    suite.addTest('Reference types toString', testReferenceToString);
    suite.addTest('Class types toString', testClassToString);
    suite.addTest('Array types toString', testArrayToString);
    return suite.run();
}

function testPrimitiveToString() {
    arktest.assertEQ('boolean', Type.of(true as boolean).toString());
    arktest.assertEQ('char', Type.of(c'a').toString());
    arktest.assertEQ('byte', Type.of(1 as byte).toString());
    arktest.assertEQ('short', Type.of(2 as short).toString());
    arktest.assertEQ('int', Type.of(3 as int).toString());
    arktest.assertEQ('long', Type.of(4 as long).toString());
    arktest.assertEQ('float', Type.of(1.0f).toString());
    arktest.assertEQ('double', Type.of(2.0 as double).toString());
    arktest.assertEQ('string', Type.of('hi').toString());
}

function testReferenceToString() {
    arktest.assertEQ('boolean', Type.of(new Boolean(true)).toString());
    arktest.assertEQ('char', Type.of(new Char(c'b')).toString());
    arktest.assertEQ('int', Type.of(new Int(10)).toString());
}

class Foo { }

function testClassToString() {
    arktest.assertEQ('std.core.Object', Type.of({} as Object).toString());
    const foo = new Foo();
    arktest.assertEQ('TypeToStringTests.Foo', Type.of(foo).toString());
}

function testArrayToString() {
    let arrInt: int[] = [];
    arktest.assertEQ('std.core.Array', Type.of(arrInt).toString());
}

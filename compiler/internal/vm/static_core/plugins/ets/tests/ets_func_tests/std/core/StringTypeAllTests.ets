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
    const suite = new arktest.ArkTestsuite('StringType API tests for SDK');

    suite.addTest('hasName returns false', testHasNameFalse);
    suite.addTest('getLiteral returns "string"', testGetLiteral);
    suite.addTest('isPrimitive returns true', testIsPrimitive);
    suite.addTest('isReference returns true', testIsReference);
    suite.addTest('equals same type', testEqualsSame);
    suite.addTest('equals different type', testEqualsDifferent);

    return suite.run();
}

function testHasNameFalse() {
    const st = Type.of('') as StringType;
    arktest.assertFalse(st.hasName(), `Expected hasName() == false`);
}

function testGetLiteral() {
    const st = Type.of('foo') as StringType;
    arktest.assertEQ(
        'string',
        st.getLiteral(),
        `Expected getLiteral() == 'string', got ${st.getLiteral()}`
    );
}

function testIsPrimitive() {
    const st = Type.of('bar') as StringType;
    arktest.assertTrue(st.isPrimitive(), `Expected isPrimitive() == true`);
}

function testIsReference() {
    const st = Type.of('baz') as StringType;
    arktest.assertTrue(st.isReference(), `Expected isReference() == true`);
}

function testEqualsSame() {
    const a = Type.of('a') as StringType;
    const b = Type.of('b') as StringType;
    arktest.assertTrue(a.equals(b), `Expected two StringType instances to be equal`);
}

function testEqualsDifferent() {
    const st = Type.of('x') as StringType;
    const it = Type.of(123 as int) as IntType;
    arktest.assertFalse(st.equals(it), `Expected StringType != IntType`);
}

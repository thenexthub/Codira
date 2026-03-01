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
    const suite = new arktest.ArkTestsuite('NullType API tests for SDK');
    suite.addTest('REF: isPrimitive/isReference/hasName', testREFProperties);
    suite.addTest('getName/getLiteral', testGetNameLiteral);
    suite.addTest('equals self', testEqualsSelf);
    suite.addTest('equals across instances', testEqualsAcross);
    return suite.run();
}

    function testREFProperties() {
    const nt = NullType.REF;
    arktest.assertTrue(nt.isPrimitive());
    arktest.assertTrue(nt.isReference());
    arktest.assertTrue(nt.hasName());
}

    function testGetNameLiteral() {
    const nt = NullType.REF;
    arktest.assertEQ('null', nt.getName());
    arktest.assertEQ('null', nt.getLiteral());
}

    function testEqualsSelf() {
    const nt = NullType.REF;
    arktest.assertTrue(nt.equals(nt));
}

    function testEqualsAcross() {
    const a = NullType.REF;
    const b = NullType.REF;
    arktest.assertTrue(a.equals(b));
    arktest.assertTrue(b.equals(a));
}

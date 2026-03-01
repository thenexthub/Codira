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

class TestClass {
    public prop1: String = '123';
    public prop2: Int = 123;
    public prop3: Byte = 12;
    public prop4: Long = 456;
    public prop5: Short = 13;
}
class TestClass1 {
    public prop1: String = '345';
    public prop2: Int = 321;
    public prop3: Byte = 14;
    public prop4: Long = 654;
    public prop5: Short = 15;
}
function test(a: TestClass | TestClass1) {
    arktest.assertEQ(a.prop1, '123', 'expected \'123\', but got ' + a.prop1);
    a.prop1 = '234'
    arktest.assertEQ(a.prop1, '234', 'expected \'234\', but got ' + a.prop1);
    arktest.assertEQ(a.prop2, 123, 'expected 123, but got ' + a.prop2);
    a.prop2 = 234
    arktest.assertEQ(a.prop2, 234, 'expected 234, but got ' + a.prop2);
    arktest.assertEQ(a.prop3, 12, 'expected 12, but got ' + a.prop3);
    a.prop3 = 13
    arktest.assertEQ(a.prop3, 13, 'expected 13, but got ' + a.prop3);
    arktest.assertEQ(a.prop4, 456, 'expected 456, but got ' + a.prop4);
    a.prop4 = 567
    arktest.assertEQ(a.prop4, 567, 'expected 567, but got ' + a.prop4);
    arktest.assertEQ(a.prop5, 13, 'expected 13, but got ' + a.prop5);
    a.prop5 = 14
    arktest.assertEQ(a.prop5, 14, 'expected 14, but got ' + a.prop5);

    return 0;
}

function main() {
    let a = new TestClass()
    return test(a)
}

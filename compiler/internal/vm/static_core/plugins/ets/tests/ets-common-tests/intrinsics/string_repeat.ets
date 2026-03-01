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


function testRepeat() {
    const Empty: String = '';
    const Hello: String = 'Hello';
    const HelloDup: String = 'HelloHelloHelloHelloHelloHelloHelloHello';
    const Nature: String = '自然界';
    const NatureDup: String = '自然界自然界自然界自然界自然界自然界自然界自然界';
    const TreeString: String = '123456789'.concat('abcdefghijklmn');
    const SlicedString: String = String('自然界12345678901234').substring(1, 16);  // 然界1234567890123
    const TreeStringDup: String = TreeString.repeat(8);
    const SlicedStringDup: String = SlicedString.repeat(8);
    arktest.assertEQ(Empty.repeat(0), '');
    arktest.assertEQ(Empty.repeat(1), '');
    arktest.assertEQ(Nature.repeat(0), '');
    arktest.assertEQ(Hello.repeat(0), '');
    arktest.assertEQ(TreeString.repeat(0), '');
    arktest.assertEQ(SlicedString.repeat(0), '');
    arktest.assertEQ(TreeString.repeat(2), '123456789abcdefghijklmn123456789abcdefghijklmn');
    arktest.assertEQ(SlicedString.repeat(2), '然界1234567890123然界1234567890123');
    arktest.assertEQ(TreeString.length * 8, TreeStringDup.length);
    arktest.assertEQ(SlicedString.length * 8, SlicedStringDup.length);
    for (let i = 1; i < 8; ++i) {
        let hello: String = Hello.repeat(i);
        let nature: String = Nature.repeat(i);
        let treeString: String = TreeString.repeat(i);
        let slicedString: String = SlicedString.repeat(i);
        arktest.assertEQ(hello, HelloDup.substring(0, 5 * i));
        arktest.assertEQ(nature, NatureDup.substring(0, 3 * i));
        arktest.assertEQ(treeString, TreeStringDup.substring(0, TreeString.length * i));
        arktest.assertEQ(slicedString, SlicedStringDup.substring(0, SlicedString.length * i));
    }

    // try repeat() with a negative index
    let errors = 0;
    try {
        let z = Empty.repeat(-1);
    } catch (e) {
        errors = 1;
    }
    arktest.assertEQ(errors, 1, 'Empty.repeat(-1) should throw an error');
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringRepeat');
    suite.addTest('testRepeat', testRepeat);
    return suite.run();
}

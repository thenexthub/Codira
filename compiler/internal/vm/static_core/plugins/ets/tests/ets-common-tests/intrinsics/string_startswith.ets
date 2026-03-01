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

function testStartsWith() {
    const Empty: String = '';
    const Esc: String = '\t\n\v';
    const Hello: String = 'Hello';
    const Nature: String = '自然界';
    const TreeString: String = '自然界12345678901234'.concat('abcdefghijklmn');
    const SlicedString: String = TreeString.substring(1, 15); // 然界123456789012
    arktest.assertTrue(TreeString.startsWith('自'));
    arktest.assertTrue(TreeString.startsWith('自然界'));
    arktest.assertTrue(TreeString.startsWith('自然界', 0));
    arktest.assertTrue(TreeString.startsWith('自然界', -1));
    arktest.assertTrue(TreeString.startsWith('然界1', 1));
    arktest.assertTrue(TreeString.startsWith('e', 21));

    arktest.assertTrue(SlicedString.startsWith('然界'));
    arktest.assertTrue(SlicedString.startsWith('然界1'));
    arktest.assertTrue(SlicedString.startsWith('然界1', 0));
    arktest.assertTrue(SlicedString.startsWith('然界1', -1));
    arktest.assertTrue(SlicedString.startsWith('界1', 1));
    arktest.assertFalse(SlicedString.startsWith('1', 0));

    arktest.assertTrue(Empty.startsWith(''));
    arktest.assertTrue(Empty.startsWith('', -1));
    arktest.assertTrue(Empty.startsWith('', 0));
    arktest.assertTrue(Empty.startsWith('', 1));
    arktest.assertFalse(Empty.startsWith('*'));
    arktest.assertFalse(Empty.startsWith('*', -1));
    arktest.assertFalse(Empty.startsWith('*', 0));
    arktest.assertFalse(Empty.startsWith('*', 1));

    arktest.assertTrue(Esc.startsWith('\t'));
    arktest.assertTrue(Esc.startsWith('\t', -1));
    arktest.assertTrue(Esc.startsWith('\t', 0));
    arktest.assertTrue(Esc.startsWith('\n', 1));
    arktest.assertTrue(Esc.startsWith('\v', 2));

    arktest.assertTrue(Hello.startsWith(''));
    arktest.assertTrue(Hello.startsWith('', -1));
    arktest.assertTrue(Hello.startsWith('', 0));
    arktest.assertTrue(Hello.startsWith('', 1));
    arktest.assertTrue(Hello.startsWith('', 2));
    arktest.assertTrue(Hello.startsWith('', 3));
    arktest.assertTrue(Hello.startsWith('', 4));
    arktest.assertTrue(Hello.startsWith('', 5));

    arktest.assertTrue(Hello.startsWith('H'));
    arktest.assertTrue(Hello.startsWith('Hel'));
    arktest.assertTrue(Hello.startsWith('Hel', 0));
    arktest.assertTrue(Hello.startsWith('Hel', -1));
    arktest.assertTrue(Hello.startsWith('ello', 1));
    arktest.assertTrue(Hello.startsWith('o', 4));

    arktest.assertTrue(Nature.startsWith('自然界'));
    arktest.assertTrue(Nature.startsWith('自然'));
    arktest.assertTrue(Nature.startsWith('自'));
    arktest.assertTrue(Nature.startsWith('自', -1));
    arktest.assertFalse(Nature.startsWith('然', 0));
    arktest.assertTrue(Nature.startsWith('然界', 1));
    arktest.assertTrue(Nature.startsWith('界', 2));
    arktest.assertFalse(Nature.startsWith('界', 3));

}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringStartsWith');
    suite.addTest('testStartsWith', testStartsWith);
    return suite.run();
}

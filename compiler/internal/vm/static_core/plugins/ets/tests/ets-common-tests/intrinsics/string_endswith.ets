/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

function testEndsWith() {
    const EMPTY: String = '';
    const HELLO: String = 'Hello';
    const NATURE: String = '自然界';
    const TreeString: String = '自然界12345678901234'.concat('abcdefghijklmn');
    const SlicedString: String = TreeString.substring(1, 15); // 然界123456789012
    arktest.assertTrue(TreeString.endsWith('lmn'));
    arktest.assertTrue(TreeString.endsWith('lmn', 35));
    arktest.assertTrue(TreeString.endsWith('lmn', 100));
    arktest.assertFalse(TreeString.endsWith('m'));

    arktest.assertTrue(SlicedString.endsWith('2'));
    arktest.assertTrue(SlicedString.endsWith('12', 15));
    arktest.assertTrue(SlicedString.endsWith('012', 100));
    arktest.assertFalse(SlicedString.endsWith('1'));

    arktest.assertTrue(EMPTY.endsWith(''));
    arktest.assertTrue(EMPTY.endsWith('', -1));
    arktest.assertTrue(EMPTY.endsWith('', 0));
    arktest.assertTrue(EMPTY.endsWith('', 1));
    arktest.assertFalse(EMPTY.endsWith('x'));
    arktest.assertFalse(EMPTY.endsWith('x', -1));
    arktest.assertFalse(EMPTY.endsWith('x', 0));
    arktest.assertFalse(EMPTY.endsWith('x', 1));
    arktest.assertFalse(EMPTY.endsWith('x', 100));

    arktest.assertTrue(HELLO.endsWith(''));
    arktest.assertTrue(HELLO.endsWith('', -1));
    arktest.assertTrue(HELLO.endsWith('', 0));
    arktest.assertTrue(HELLO.endsWith('', 1));
    arktest.assertTrue(HELLO.endsWith('', 5));
    arktest.assertTrue(HELLO.endsWith('', 100));

    arktest.assertTrue(HELLO.endsWith('o'));
    arktest.assertFalse(HELLO.endsWith('o', -1));
    arktest.assertFalse(HELLO.endsWith('o', 0));
    arktest.assertFalse(HELLO.endsWith('o', 1));
    arktest.assertFalse(HELLO.endsWith('o', 2));
    arktest.assertFalse(HELLO.endsWith('o', 3));
    arktest.assertFalse(HELLO.endsWith('o', 4));
    arktest.assertTrue(HELLO.endsWith('o', 5));
    arktest.assertTrue(HELLO.endsWith('o', 100));

    arktest.assertTrue(HELLO.endsWith('ello'));
    arktest.assertFalse(HELLO.endsWith('ello', -1));
    arktest.assertFalse(HELLO.endsWith('ello', 0));
    arktest.assertFalse(HELLO.endsWith('ello', 1));
    arktest.assertFalse(HELLO.endsWith('ello', 2));
    arktest.assertFalse(HELLO.endsWith('ello', 3));
    arktest.assertFalse(HELLO.endsWith('ello', 4));
    arktest.assertTrue(HELLO.endsWith('ello', 5));
    arktest.assertTrue(HELLO.endsWith('ello', 100));

    arktest.assertTrue(HELLO.endsWith('Hello'));
    arktest.assertFalse(HELLO.endsWith('Hello', -1));
    arktest.assertFalse(HELLO.endsWith('Hello', 0));
    arktest.assertFalse(HELLO.endsWith('Hello', 1));
    arktest.assertFalse(HELLO.endsWith('Hello', 2));
    arktest.assertFalse(HELLO.endsWith('Hello', 3));
    arktest.assertFalse(HELLO.endsWith('Hello', 4));
    arktest.assertTrue(HELLO.endsWith('Hello', 5));
    arktest.assertTrue(HELLO.endsWith('Hello', 100));

    arktest.assertTrue(NATURE.endsWith(''));
    arktest.assertTrue(NATURE.endsWith('', -1));
    arktest.assertTrue(NATURE.endsWith('', 0));
    arktest.assertTrue(NATURE.endsWith('', 1));
    arktest.assertTrue(NATURE.endsWith('', 2));
    arktest.assertTrue(NATURE.endsWith('', 3));
    arktest.assertTrue(NATURE.endsWith('', 100));

    arktest.assertTrue(NATURE.endsWith('然界'));
    arktest.assertFalse(NATURE.endsWith('然界', -1));
    arktest.assertFalse(NATURE.endsWith('然界', 0));
    arktest.assertFalse(NATURE.endsWith('然界', 1));
    arktest.assertFalse(NATURE.endsWith('然界', 2));
    arktest.assertTrue(NATURE.endsWith('然界', 3));
    arktest.assertTrue(NATURE.endsWith('然界', 100));

    arktest.assertTrue(NATURE.endsWith('自然界'));
    arktest.assertFalse(NATURE.endsWith('自然界', -1));
    arktest.assertFalse(NATURE.endsWith('自然界', 0));
    arktest.assertFalse(NATURE.endsWith('自然界', 1));
    arktest.assertFalse(NATURE.endsWith('自然界', 2));
    arktest.assertTrue(NATURE.endsWith('自然界', 100));
}

function main(): int {
    let suite = new arktest.ArkTestsuite('StringEndsWith');
    suite.addTest('testEndsWith', testEndsWith);
    return suite.run();
}
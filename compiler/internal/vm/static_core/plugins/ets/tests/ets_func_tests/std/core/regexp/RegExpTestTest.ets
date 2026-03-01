/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function test(): void {
    arktest.assertEQ(RegExp("abacaba").test("abacaba"), true)
    arktest.assertEQ(RegExp("abacaba").test("abacab_"), false)
    arktest.assertEQ(RegExp("(a.){4}").test("abacaba"), false)
    arktest.assertEQ(RegExp("(a.?){4}").test("abacaba"), true)
    arktest.assertEQ(RegExp("").test(""), true)
    arktest.assertEQ(RegExp("\u{55555}", "u").test("\u{55555}"), true)
    arktest.assertEQ(RegExp("\u{55555}", "u").test("abacaba"), false)
    arktest.assertEQ(RegExp("\u{12345}{3}", "u").test("\u{12345}\udf45\udf45"), false)
    arktest.assertEQ(RegExp("\u{12345}{3}").test("\u{12345}\udf45\udf45"), true)
    arktest.assertEQ(RegExp("\\u0410").test("\\u0410"), false)
    arktest.assertEQ(RegExp("\\u0410").test("\u0410"), true)
    arktest.assertEQ(RegExp("\u0410").test("\\u0410"), false)
    arktest.assertEQ(RegExp("\u0410").test("\u0410"), true)
    arktest.assertEQ(RegExp("\\u0410").test("bb\\u0410"), false)
    arktest.assertEQ(RegExp("\\u0410").test("bb\u0410"), true)
    arktest.assertEQ(RegExp("\u0410").test("bb\\u0410"), false)
    arktest.assertEQ(RegExp("\u0410").test("bb\u0410"), true)
    arktest.assertEQ(RegExp("\\u0410").test("bb\\u0410bb"), false)
    arktest.assertEQ(RegExp("\\u0410").test("bb\u0410bb"), true)
    arktest.assertEQ(RegExp("\u0410").test("bb\\u0410bb"), false)
    arktest.assertEQ(RegExp("\u0410").test("bb\u0410bb"), true)
    arktest.assertEQ(RegExp("\\u0410").test("\\u0410bb"), false)
    arktest.assertEQ(RegExp("\\u0410").test("\u0410bb"), true)
    arktest.assertEQ(RegExp("\u0410").test("\\u0410bb"), false)
    arktest.assertEQ(RegExp("\u0410").test("\u0410bb"), true)

    arktest.assertEQ(RegExp("\\1(a)").test("aa"), true)
}

function main(): int {
    const suite = new arktest.ArkTestsuite("RegExp.test")

    suite.addTest('test', test)
    suite.addTest('RegExp.testSurrogate', testSurrogate)
    suite.addTest("CaselessTest", caselessTest)

    return suite.run()
}

function testSurrogate(): void {
    let regex = new RegExp('\uD800', 'u');
    let result = regex.exec('\uD800');
    arktest.assertEQ(result?.[0], '\uD800');

    regex = new RegExp('\uD800', 'gmu');
    result = regex.exec('\uD800');
    arktest.assertEQ(result?.[0], '\uD800');
}

function caselessTest(): void {
    let regex = new RegExp('\u00C7', 'i');
    arktest.assertTrue(regex.test('ç'), "RegExp('\u00C7', 'i').test('ç') should be true");
    arktest.assertTrue(regex.test('Ç'), "RegExp('\u00C7', 'i').test('Ç') should be true");
    arktest.assertFalse(regex.test('c'), "RegExp('\u00C7', 'i').test('c') should be false");

    regex = new RegExp('Z', 'i');
    arktest.assertTrue(regex.test('z'), "RegExp('Z', 'i').test('z') should be true");
    arktest.assertTrue(regex.test('Z'), "RegExp('Z', 'i').test('Z') should be true");
    arktest.assertFalse(regex.test('Ż'), "RegExp('Z', 'i').test('Ż') should be false");
    arktest.assertFalse(regex.test('ż'), "RegExp('Z', 'i').test('ż') should be false");

    regex = new RegExp('\u017B', 'i');
    arktest.assertTrue(regex.test('ż'), "RegExp('\u017B', 'i').test('ż') should be true");
    arktest.assertTrue(regex.test('Ż'), "RegExp('\u017B', 'i').test('Ż') should be true");
    arktest.assertFalse(regex.test('z'), "RegExp('\u017B', 'i').test('z') should be false");
}
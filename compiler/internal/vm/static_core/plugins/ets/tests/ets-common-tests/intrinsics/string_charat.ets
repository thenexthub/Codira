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

let EscChars: String = '\f\n\r\t\v';
const RusChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя'; // 16-bit
const EngChars: String = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz';               //  8-bit
const Symbols: String = '`~!@#$%^&*()_-+={}[]:;\"\'\\|/<>,.';
const RUS_LEN: int = RusChars.getLength();
const ENG_LEN: int = EngChars.getLength();
const SYM_LEN: int = Symbols.getLength();

function testLineStringCharAt() {
    arktest.assertEQ(EscChars.charAt(0), c'\f');
    arktest.assertEQ(EscChars.charAt(1), c'\n');
    arktest.assertEQ(EscChars.charAt(2), c'\r');
    arktest.assertEQ(EscChars.charAt(3), c'\t');
    arktest.assertEQ(EscChars.charAt(4), c'\v');

    arktest.assertEQ(RusChars.charAt(0), c'А');
    arktest.assertEQ(RusChars.charAt(1), c'а');
    arktest.assertEQ(RusChars.charAt(2), c'Б');
    arktest.assertEQ(RusChars.charAt(RUS_LEN - 1), c'я');
    arktest.assertEQ(RusChars.charAt(RUS_LEN - 2), c'Я');

    arktest.assertEQ(EngChars.charAt(0), c'A');
    arktest.assertEQ(EngChars.charAt(1), c'a');
    arktest.assertEQ(EngChars.charAt(2), c'B');
    arktest.assertEQ(EngChars.charAt(ENG_LEN - 1), c'z');

    arktest.assertEQ(Symbols.charAt(0), c'`');
    arktest.assertEQ(Symbols.charAt(1), c'~');
    arktest.assertEQ(Symbols.charAt(SYM_LEN - 1), c'.');
    arktest.assertEQ(Symbols.charAt(SYM_LEN - 2), c',');
}

function testTreeStringCharAt() {
    let TreeUtf8Chars = EscChars.concat(EngChars);
    arktest.assertEQ(TreeUtf8Chars.charAt(0), c'\f');
    arktest.assertEQ(TreeUtf8Chars.charAt(1), c'\n');
    arktest.assertEQ(TreeUtf8Chars.charAt(5), c'A');
    arktest.assertEQ(TreeUtf8Chars.charAt(6), c'a');
    arktest.assertEQ(TreeUtf8Chars.charAt(TreeUtf8Chars.getLength() - 1), c'z');
    arktest.assertEQ(TreeUtf8Chars.charAt(TreeUtf8Chars.getLength() - 2), c'Z');

    let rus_0 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМм';
    let rus_1 = 'НнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let TreeUtf16Chars = rus_0.concat(rus_1);
    arktest.assertEQ(TreeUtf16Chars.charAt(0), c'А');
    arktest.assertEQ(TreeUtf16Chars.charAt(1), c'а');
    arktest.assertEQ(TreeUtf16Chars.charAt(2), c'Б');
    arktest.assertEQ(TreeUtf16Chars.charAt(3), c'б');
    arktest.assertEQ(TreeUtf16Chars.charAt(4), c'В');
    arktest.assertEQ(TreeUtf16Chars.charAt(5), c'в');
    arktest.assertEQ(TreeUtf16Chars.charAt(rus_0.getLength()), c'Н');
    arktest.assertEQ(TreeUtf16Chars.charAt(rus_0.getLength() - 1), c'м');
    arktest.assertEQ(TreeUtf16Chars.charAt(TreeUtf16Chars.getLength() - 1), c'я');

}

function testSlicedStringCharAt() {
    let TreeUtf8Chars = EscChars.concat(EngChars);
    let slicedUtf8Chars = TreeUtf8Chars.substring(1, 16);
    arktest.assertEQ(slicedUtf8Chars.charAt(0), c'\n');
    arktest.assertEQ(slicedUtf8Chars.charAt(1), c'\r');
    arktest.assertEQ(slicedUtf8Chars.charAt(4), c'A');
    arktest.assertEQ(slicedUtf8Chars.charAt(5), c'a');
    arktest.assertEQ(slicedUtf8Chars.charAt(slicedUtf8Chars.getLength() - 1), c'F');
    let rus_0 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМм';
    let rus_1 = 'НнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let TreeUtf16Chars = rus_0.concat(rus_1);
    let slicedUtf16Chars = TreeUtf16Chars.substring(rus_0.getLength(), TreeUtf16Chars.getLength());
    arktest.assertEQ(slicedUtf16Chars.charAt(0), c'Н');
    arktest.assertEQ(slicedUtf16Chars.charAt(1), c'н');
    arktest.assertEQ(slicedUtf16Chars.charAt(slicedUtf16Chars.getLength() - 1), c'я');
    arktest.assertEQ(slicedUtf16Chars.charAt(slicedUtf16Chars.getLength() - 2), c'Я');
    arktest.assertEQ(slicedUtf16Chars, rus_1);
    arktest.assertEQ(rus_1, slicedUtf16Chars);
}

function main(): int {
    /**
     * charAt(int): char => StdCoreStringCharAt
     */
    let suite = new arktest.ArkTestsuite('StringCharAt');
    suite.addTest('testLineStringCharAt', testLineStringCharAt);
    suite.addTest('testTreeStringCharAt', testTreeStringCharAt);
    suite.addTest('testSlicedStringCharAt', testSlicedStringCharAt);
    return suite.run();
}

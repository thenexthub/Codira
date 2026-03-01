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

let Empty: String = '';
let EscChars: String = '\f\n\r\t\v';
let RusChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя'; // 16-bit
let EngChars: String = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz';               //  8-bit
let DupChars: String = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';               //  8-bit
const Digits: String = '0123456789';
const Symbols: String = '`~!@#$%^&*()_-+={}[]:;\"\'\\|/<>,.';
const CharNotFound: int = -1;

function main(): int {

    /**
     * lastIndexOf(string): int => StdCoreStringlastIndexOfString
     */

    arktest.assertEQ(Empty.lastIndexOf(''), 0);
    arktest.assertEQ(Empty.lastIndexOf('x'), CharNotFound);
    arktest.assertEQ(EscChars.lastIndexOf('\r\t\v'), 2);
    arktest.assertEQ(EscChars.lastIndexOf(EscChars), 0);
    for (let i = 0; i < EscChars.getLength(); ++i) {
        arktest.assertEQ(EscChars.lastIndexOf(EscChars.substring(i, EscChars.length)), i);
    }
    for (let i = 1; i < EscChars.getLength(); ++i) {
        arktest.assertEQ(EscChars.lastIndexOf(EscChars.substring(0, i), i + 1), 0);
    }
    let mixedTreeChars = EscChars.concat(EngChars);
    for (let i = 0; i < mixedTreeChars.getLength(); ++i) {
        arktest.assertEQ(mixedTreeChars.lastIndexOf(mixedTreeChars.substring(i, mixedTreeChars.length)), i);
    }
    for (let i = 1; i < mixedTreeChars.getLength(); ++i) {
        arktest.assertEQ(mixedTreeChars.lastIndexOf(mixedTreeChars.substring(0, i), i + 1), 0);
    }
    let escSlicedChars = EscChars.substring(1, 18);
    for (let i = 0; i < escSlicedChars.getLength(); ++i) {
        arktest.assertEQ(escSlicedChars.lastIndexOf(escSlicedChars.substring(i, escSlicedChars.length)), i);
    }
    for (let i = 1; i < escSlicedChars.getLength(); ++i) {
        arktest.assertEQ(escSlicedChars.lastIndexOf(escSlicedChars.substring(0, i), i + 1), 0);
    }

    for (let i = 0; i < RusChars.getLength(); ++i) {
        arktest.assertEQ(RusChars.lastIndexOf(RusChars.substring(i, RusChars.length)), i);
    }
    for (let i = 1; i < RusChars.getLength(); ++i) {
        arktest.assertEQ(RusChars.lastIndexOf(RusChars.substring(0, i), i + 1), 0);
    }
    arktest.assertEQ(RusChars.lastIndexOf('Ѫ'), CharNotFound);
    arktest.assertEQ(RusChars.lastIndexOf(RusChars), 0);

    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    arktest.assertTrue(rusTreeChars === RusChars);
    arktest.assertTrue(RusChars === rusTreeChars);
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_1), 0);
    arktest.assertEQ(RusChars.lastIndexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_1), 0);
    arktest.assertEQ(rusTreeChars.lastIndexOf(rusChars_2), rusChars_1.length);
    for (let i = 0; i < rusTreeChars.getLength(); ++i) {
        arktest.assertEQ(rusTreeChars.lastIndexOf(rusTreeChars.substring(i, rusTreeChars.length)), i);
    }
    for (let i = 1; i < rusTreeChars.getLength(); ++i) {
        arktest.assertEQ(rusTreeChars.lastIndexOf(rusTreeChars.substring(0, i), i + 1), 0);
    }
    let rusSlicedChars = rusTreeChars.substring(3, 30);
    arktest.assertEQ(RusChars.lastIndexOf(rusSlicedChars), 3);
    for (let i = 0; i < rusSlicedChars.getLength(); ++i) {
        arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars.substring(i, rusSlicedChars.length)), i);
    }
    for (let i = 1; i < rusSlicedChars.getLength(); ++i) {
        arktest.assertEQ(rusSlicedChars.lastIndexOf(rusSlicedChars.substring(0, i), i + 1), 0);
    }

    for (let i = 0; i < EngChars.getLength(); ++i) {
        arktest.assertEQ(EngChars.lastIndexOf(EngChars.substring(i, EngChars.length)), i);
    }
    for (let i = 1; i < EngChars.getLength(); ++i) {
        arktest.assertEQ(EngChars.lastIndexOf(EngChars.substring(0, i), i + 1), 0);
    }
    arktest.assertEQ(EngChars.lastIndexOf(c'æ'), CharNotFound);
    arktest.assertEQ(EngChars.lastIndexOf(EngChars), 0);

    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    arktest.assertEQ(EngChars.lastIndexOf(engChars_1), 0);
    arktest.assertEQ(EngChars.lastIndexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_1), 0);
    arktest.assertEQ(engTreeChars.lastIndexOf(engChars_2), engChars_1.length);
    arktest.assertTrue(engTreeChars === EngChars);
    arktest.assertTrue(EngChars === engTreeChars);
    for (let i = 0; i < engTreeChars.getLength(); ++i) {
        arktest.assertEQ(engTreeChars.lastIndexOf(engTreeChars.substring(i, engTreeChars.length)), i);
    }
    for (let i = 1; i < engTreeChars.getLength(); ++i) {
        arktest.assertEQ(engTreeChars.lastIndexOf(engTreeChars.substring(0, i), i + 1), 0);
    }
    let engSlicedChars = engTreeChars.substring(3, 30);
    for (let i = 0; i < engSlicedChars.getLength(); ++i) {
        arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars.substring(i, engSlicedChars.length)), i);
    }
    for (let i = 1; i < engSlicedChars.getLength(); ++i) {
        arktest.assertEQ(engSlicedChars.lastIndexOf(engSlicedChars.substring(0, i), i + 1), 0);
    }

    for (let i = 0; i < DupChars.getLength(); ++i) {
        arktest.assertEQ(DupChars.lastIndexOf(DupChars.substring(i, DupChars.length)), i);
    }
    for (let i = 1; i < DupChars.getLength(); ++i) {
        arktest.assertEQ(DupChars.lastIndexOf(DupChars.substring(0, i)), DupChars.length - i);
    }
    for (let i = 0; i < Digits.getLength(); ++i) {
        arktest.assertEQ(Digits.lastIndexOf(Digits.substring(i, Digits.length)), i);
    }
    for (let i = 1; i < Digits.getLength(); ++i) {
        arktest.assertEQ(Digits.lastIndexOf(Digits.substring(0, i), i + 1), 0);
    }
    for (let i = 0; i < Symbols.getLength(); ++i) {
        arktest.assertEQ(Symbols.lastIndexOf(Symbols.substring(i, Symbols.length)), i);
    }
    for (let i = 1; i < Symbols.getLength(); ++i) {
        arktest.assertEQ(Symbols.lastIndexOf(Symbols.substring(0, i), i + 1), 0);
    }
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    for (let i = 0; i < symbolsTreeChars.getLength(); ++i) {
        arktest.assertEQ(symbolsTreeChars.lastIndexOf(symbolsTreeChars.substring(i, symbolsTreeChars.length)), i);
    }
    let symbolsSlicedChars = symbolsTreeChars.substring(1, 12);
    for (let i = 1; i < symbolsSlicedChars.getLength(); ++i) {
        arktest.assertEQ(symbolsSlicedChars.lastIndexOf(symbolsSlicedChars.substring(0, i), i + 1), 0);
    }
    return 0;
}

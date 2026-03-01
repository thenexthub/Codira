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
     * indexOf(string): int => StdCoreStringIndexOfString
     */

    arktest.assertEQ(Empty.indexOf('', 0), 0);
    arktest.assertEQ(Empty.indexOf('x', 0), CharNotFound);
    arktest.assertEQ(EscChars.indexOf('\r\t\v', 1), 2);
    arktest.assertEQ(EscChars.indexOf(EscChars), 0);
    for (let i = 0; i < EscChars.getLength(); ++i) {
        arktest.assertEQ(EscChars.indexOf(EscChars.substring(i, EscChars.length), i - 1), i);
    }
    for (let i = 0; i < EscChars.getLength(); ++i) {
        arktest.assertEQ(EscChars.indexOf(EscChars.substring(0, i), 0), 0);
    }
    let mixedTreeChars = EscChars.concat(EngChars);
    for (let i = 0; i < mixedTreeChars.getLength(); ++i) {
        arktest.assertEQ(mixedTreeChars.indexOf(mixedTreeChars.substring(i, mixedTreeChars.length), i - 1), i);
    }
    for (let i = 0; i < mixedTreeChars.getLength(); ++i) {
        arktest.assertEQ(mixedTreeChars.indexOf(mixedTreeChars.substring(0, i), 0), 0);
    }
    let escSlicedChars = EscChars.substring(1, 20);
    for (let i = 0; i < escSlicedChars.getLength(); ++i) {
        arktest.assertEQ(escSlicedChars.indexOf(escSlicedChars.substring(i, escSlicedChars.length), i - 1), i);
    }
    for (let i = 0; i < escSlicedChars.getLength(); ++i) {
        arktest.assertEQ(escSlicedChars.indexOf(escSlicedChars.substring(0, i), 0), 0);
    }

    for (let i = 0; i < RusChars.getLength(); ++i) {
        arktest.assertEQ(RusChars.indexOf(RusChars.substring(i, RusChars.length), i - 1), i);
    }
    for (let i = 0; i < RusChars.getLength(); ++i) {
        arktest.assertEQ(RusChars.indexOf(RusChars.substring(0, i), 0), 0);
    }
    arktest.assertEQ(RusChars.indexOf('Ѫ'), CharNotFound);
    arktest.assertEQ(RusChars.indexOf(RusChars), 0);

    let rusChars_1 = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНн';
    let rusChars_2 = 'ОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let rusTreeChars = rusChars_1.concat(rusChars_2);
    arktest.assertTrue(rusTreeChars === RusChars);
    arktest.assertTrue(RusChars === rusTreeChars);
    arktest.assertEQ(RusChars.indexOf(rusChars_1), 0);
    arktest.assertEQ(RusChars.indexOf(rusChars_2), rusChars_1.length);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_1), 0);
    arktest.assertEQ(rusTreeChars.indexOf(rusChars_2), rusChars_1.length);
    for (let i = 0; i < rusTreeChars.getLength(); ++i) {
        arktest.assertEQ(rusTreeChars.indexOf(rusTreeChars.substring(i, rusTreeChars.length), i - 1), i);
    }
    for (let i = 0; i < rusTreeChars.getLength(); ++i) {
        arktest.assertEQ(rusTreeChars.indexOf(rusTreeChars.substring(0, i), 0), 0);
    }
    let rusSlicedChars = rusTreeChars.substring(3, 30);
    arktest.assertEQ(RusChars.indexOf(rusSlicedChars), 3);
    for (let i = 0; i < rusSlicedChars.getLength(); ++i) {
        arktest.assertEQ(rusSlicedChars.indexOf(rusSlicedChars.substring(i, rusSlicedChars.length), i - 1), i);
    }
    for (let i = 0; i < rusSlicedChars.getLength(); ++i) {
        arktest.assertEQ(rusSlicedChars.indexOf(rusSlicedChars.substring(0, i), 0), 0);
    }

    for (let i = 0; i < EngChars.getLength(); ++i) {
        arktest.assertEQ(EngChars.indexOf(EngChars.substring(i, EngChars.length), i - 1), i);
    }
    for (let i = 0; i < EngChars.getLength(); ++i) {
        arktest.assertEQ(EngChars.indexOf(EngChars.substring(0, i), 0), 0);
    }
    arktest.assertEQ(EngChars.indexOf(c'æ'), CharNotFound);
    arktest.assertEQ(EngChars.indexOf(EngChars), 0);

    let engChars_1 = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNn';
    let engChars_2 = 'OoPpQqRrSsTtUuVvWwXxYyZz';
    let engTreeChars = engChars_1.concat(engChars_2);
    arktest.assertEQ(EngChars.indexOf(engChars_1), 0);
    arktest.assertEQ(EngChars.indexOf(engChars_2), engChars_1.length);
    arktest.assertEQ(engTreeChars.indexOf(engChars_1), 0);
    arktest.assertEQ(engTreeChars.indexOf(engChars_2), engChars_1.length);
    arktest.assertTrue(engTreeChars === EngChars);
    arktest.assertTrue(EngChars === engTreeChars);
    for (let i = 0; i < engTreeChars.getLength(); ++i) {
        arktest.assertEQ(engTreeChars.indexOf(engTreeChars.substring(i, engTreeChars.length), i - 1), i);
    }
    for (let i = 0; i < engTreeChars.getLength(); ++i) {
        arktest.assertEQ(engTreeChars.indexOf(engTreeChars.substring(0, i), 0), 0);
    }
    let engSlicedChars = engTreeChars.substring(3, 30);
    for (let i = 0; i < engSlicedChars.getLength(); ++i) {
        arktest.assertEQ(engSlicedChars.indexOf(engSlicedChars.substring(i, engSlicedChars.length), i - 1), i);
    }
    for (let i = 0; i < engSlicedChars.getLength(); ++i) {
        arktest.assertEQ(engSlicedChars.indexOf(engSlicedChars.substring(0, i), 0), 0);
    }

    for (let i = 1; i < DupChars.getLength(); ++i) {
        arktest.assertEQ(DupChars.indexOf(DupChars.substring(i, DupChars.length), i - 1), i - 1);
    }
    for (let i = 0; i < DupChars.getLength(); ++i) {
        arktest.assertEQ(DupChars.indexOf(DupChars.substring(0, i), 0), 0);
    }
    for (let i = 0; i < Digits.getLength(); ++i) {
        arktest.assertEQ(Digits.indexOf(Digits.substring(i, Digits.length), i - 1), i);
    }
    for (let i = 0; i < Digits.getLength(); ++i) {
        arktest.assertEQ(Digits.indexOf(Digits.substring(0, i), 0), 0);
    }
    for (let i = 0; i < Symbols.getLength(); ++i) {
        arktest.assertEQ(Symbols.indexOf(Symbols.substring(i, Symbols.length), i - 1), i);
    }
    for (let i = 0; i < Symbols.getLength(); ++i) {
        arktest.assertEQ(Symbols.indexOf(Symbols.substring(0, i), 0), 0);
    }
    let symbols_1 = '`~!@#$%^&*()_-+={}[]';
    let symbols_2 = ':;\"\'\\|/<>,.';
    let symbolsTreeChars = symbols_1.concat(symbols_2);
    for (let i = 0; i < symbolsTreeChars.getLength(); ++i) {
        arktest.assertEQ(symbolsTreeChars.indexOf(symbolsTreeChars.substring(i, symbolsTreeChars.length), i - 1), i);
    }
    let symbolsSlicedChars = symbolsTreeChars.substring(1, 14);
    for (let i = 0; i < symbolsSlicedChars.getLength(); ++i) {
        arktest.assertEQ(symbolsSlicedChars.indexOf(symbolsSlicedChars.substring(i, symbolsSlicedChars.length), i - 1), i);
    }
    return 0;
}

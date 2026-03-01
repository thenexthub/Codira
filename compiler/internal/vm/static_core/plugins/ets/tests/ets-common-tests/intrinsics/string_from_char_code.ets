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

function emptyStringTest() {
    arktest.assertEQ(string.fromCharCode(), '', 'Must be an empty string');
    arktest.assertEQ(string.fromCharCode().length, 0, 'String length of an empty string must be equal to 0');
}

function compressibleStringTest() {
    arktest.assertEQ(string.fromCharCode(0x43, 65), 'CA', "Must be 'CA'");
    arktest.assertEQ(string.fromCharCode(0x43, 65).length, 2, "String length of 'CA' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(0xA, 0xA), '\n\n', "Must be '\\n\\n'");
    arktest.assertEQ(string.fromCharCode(0xA, 0xA).length, 2, "String length of '\\n\\n' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(0xA, 0x9, 0xA), '\n\t\n', "Must be '\\n\\t\\n'");
    arktest.assertEQ(string.fromCharCode(0xA, 0x9, 0xA).length, 3, "String length of '\\n\\t\\n' must be equal to 3");
    arktest.assertEQ(string.fromCharCode(0xA, 0xD), '\n\r', "Must be '\\n\\r'");
    arktest.assertEQ(string.fromCharCode(0xA, 0xD).length, 2, "String length of '\\n\\r' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44), 'ABCD', "Must be 'ABCD'");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44).length, 4, "String length of 'ABCD' must be equal to 4");
}

function compressibleSingleStringTest() {
    arktest.assertEQ(string.fromCharCode(65), 'A', "Must be 'A'");
    arktest.assertEQ(string.fromCharCode(65).length, 1, "String length of 'A' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x41), 'A', "Must be 'A'");
    arktest.assertEQ(string.fromCharCode(0x41).length, 1, "String length of 'A' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x43), 'C', "Must be 'C'");
    arktest.assertEQ(string.fromCharCode(0x43).length, 1, "String length of 'C' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x9), '\t', "Must be '\\t'");
    arktest.assertEQ(string.fromCharCode(0x9).length, 1, "String length of '\\t' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0xA), '\n', "Must be '\\n'");
    arktest.assertEQ(string.fromCharCode(0xA).length, 1, "String length of '\\n' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0xA, 0xD), '\n\r', "Must be '\\n\\r'");
    arktest.assertEQ(string.fromCharCode(0xA, 0xD).length, 2, "String length of '\\n\\r' must be equal to 2");
}

function incompressibleStringTest() {
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x41, 0x42, 0x43, 0x44), 'αABCD', "Must be 'αABCD'");
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x41, 0x42, 0x43, 0x44).length, 5, "String length of 'αABCD' must be equal to 5");
    arktest.assertEQ(string.fromCharCode(0x41, 0x3B1, 0x42, 0x43, 0x44), 'AαBCD', "Must be 'AαBCD'");
    arktest.assertEQ(string.fromCharCode(0x41, 0x3B1, 0x42, 0x43, 0x44).length, 5, "String length of 'AαBCD' must be equal to 5");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44, 0x3B1), 'ABCDα', "Must be 'ABCDα'");
    arktest.assertEQ(string.fromCharCode(0x41, 0x42, 0x43, 0x44, 0x3B1).length, 5, "String length of 'ABCDα' must be equal to 5");
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x3B2, 0x3B3), 'αβγ', "Must be 'αβγ'");
    arktest.assertEQ(string.fromCharCode(0x3B1, 0x3B2, 0x3B3).length, 3, "String length of 'αβγ' must be equal to 3");
    arktest.assertEQ(string.fromCharCode(0x43, 0x3B2, 0xD799,
                                 0x1D798, 65),
        'Cβ힙힘A', "Must be 'Cβ힙힘A'");
    arktest.assertEQ(string.fromCharCode(65435, 65435), 'ﾛﾛ', "Error! Must be 'ﾛﾛ'");
    arktest.assertEQ(string.fromCharCode(65436, 65436), 'ﾜﾜ', "Must be 'ﾜﾜ'");
    arktest.assertEQ(string.fromCharCode(65437, 65437), 'ﾝﾝ', "Error! Must be 'ﾝﾝ'");
}

function incompressibleSingleStringTest() {
    arktest.assertEQ(string.fromCharCode(0x3B2), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(0x3B2).length, 1, "String length of 'β' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0x103B2), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(0x103B2).length, 1, "String length of 'β' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(0xD799), '힙', "Must be '힙'");
    arktest.assertEQ(string.fromCharCode(0xD799).length, 1, "String length of '힙' must be equal to 1");
    arktest.assertEQ(string.fromCharCode(65435), 'ﾛ', "Error! Must be 'ﾛ'");
    arktest.assertEQ(string.fromCharCode(65436), 'ﾜ', "Must be 'ﾜ'");
    arktest.assertEQ(string.fromCharCode(65437), 'ﾝ', "Error! Must be 'ﾝ'");
}

function falsePositiveWrongSymbolFlagTest() {
    arktest.assertEQ(string.fromCharCode(0x1000003B2, 0x1000003B2), 'ββ', "Must be 'ββ'");
    arktest.assertEQ(string.fromCharCode(0x1000003B2, 0x1000003B2).length, 2, "String length of 'ββ' must be equal to 2");
}

function falsePositiveWrongSymbolFlagSingleTest() {
    arktest.assertEQ(string.fromCharCode(0x1000003B2), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(0x1000003B2).length, 1, "String length of 'β' must be equal to 1");
}

function negativeCasesTest() {
    arktest.assertEQ(string.fromCharCode(-100), 'ﾜ', "Must be 'ﾜ'");
    arktest.assertEQ(string.fromCharCode(-100).length, 1);
    arktest.assertEQ(string.fromCharCode(-65535), '\u{1}', 'String built from -65535 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535).length, 1);
    arktest.assertEQ(string.fromCharCode(-65535, -65535), '\u{1}\u{1}', 'String built from -65535 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535, -65535).length, 2);
    arktest.assertEQ(string.fromCharCode(-65535, -100), '\u{1}ﾜ', 'String built from -65535, -100 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535, -100).length, 2);
    arktest.assertEQ(string.fromCharCode(-100, -65535), 'ﾜ\u{1}', 'String built from -100, -65535 must be ﾜ');
    arktest.assertEQ(string.fromCharCode(-100, -65535).length, 2);
    arktest.assertEQ(string.fromCharCode(-65536, -65536), '\u{0}\u{0}', 'String built from -65536 must be 0');
    arktest.assertEQ(string.fromCharCode(-65536, -65536).length, 2);
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100), 'fﾜ', "Must be 'fﾜ'");
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100).length, 2, "String length of 'fﾜ' must be equal to 2");
    arktest.assertEQ(string.fromCharCode(-100, 0xffff0066), 'ﾜf', "Must be 'ﾜf'");
    arktest.assertEQ(string.fromCharCode(-100, 0xffff0066).length, 2, "String length of 'ﾜf' must be equal to 2");
}

function negativeSingleCasesTest() {
    arktest.assertEQ(string.fromCharCode(-100), 'ﾜ', "Must be 'ﾜ'");
    arktest.assertEQ(string.fromCharCode(-100).length, 1);
    arktest.assertEQ(string.fromCharCode(-65535), '\u{1}', 'String built from -65535 must be 1');
    arktest.assertEQ(string.fromCharCode(-65535).length, 1);
    arktest.assertEQ(string.fromCharCode(-65536), '\u{0}', 'String built from -65536 must be 0');
    arktest.assertEQ(string.fromCharCode(-65536).length, 1);
    arktest.assertEQ(string.fromCharCode(-1), '\u{ffff}', 'String built from -1 must be 65535');
    arktest.assertEQ(string.fromCharCode(-1).length, 1);
    arktest.assertEQ(string.fromCharCode(0xffff0066), 'f', "Must be 'f'");
    arktest.assertEQ(string.fromCharCode(0xffff0066).length, 1, "String length of 'f' must be equal to 1");
}

function saturationCasesTest() {
    arktest.assertEQ(string.fromCharCode(65535, 65535), '\u{ffff}\u{ffff}', 'String built from 65535 must be 65535');
    arktest.assertEQ(string.fromCharCode(65535, 65535).length, 2);
    arktest.assertEQ(string.fromCharCode(65536, 65536), '\u{0}\u{0}', 'String built from 65536 must be 0');
    arktest.assertEQ(string.fromCharCode(65536, 65536).length, 2);
    arktest.assertEQ(string.fromCharCode((4294901862).toInt(), (4294901862).toInt()), 'ff', "Must be 'ff'");
    arktest.assertEQ(string.fromCharCode((4294901862).toInt(), (4294901862).toInt()).length, 2, "String length of 'ff' must be equal to 2");
}

function saturationSingleCasesTest() {
    arktest.assertEQ(string.fromCharCode(65535), '\u{ffff}', 'String built from 65535 must be 65535');
    arktest.assertEQ(string.fromCharCode(65535).length, 1);
    arktest.assertEQ(string.fromCharCode(65536), '\u{0}', 'String built from 65536 must be 0');
    arktest.assertEQ(string.fromCharCode(65536).length, 1);
    arktest.assertEQ(string.fromCharCode((4294901862).toInt()), 'f', "Must be 'f'");
    arktest.assertEQ(string.fromCharCode((4294901862).toInt()).length, 1, "String length of 'f' must be equal to 1");
}

function overflowCasesTest() {
    const MAX = 65536
    arktest.assertEQ(string.fromCharCode(MAX, MAX - 1), '\u{0}\u{ffff}',
        'String built from char codes [MAX, MAX - 1] must be \u{0}\u{ffff}');
    arktest.assertEQ(string.fromCharCode(MAX - 1, MAX), '\u{ffff}\u{0}',
        'String built from char codes [MAX - 1, MAX] must be \u{ffff}\u{0}');
    arktest.assertEQ(String.fromCharCode(-MAX, -MAX + 1), '\u{0}\u{1}',
        'String built from char codes [-MAX, -MAX + 1] must be \u{0}\u{1}');
    arktest.assertEQ(String.fromCharCode(-MAX + 1, -MAX), '\u{1}\u{0}',
        'String built from char codes [-MAX + 1, -MAX] must be \u{1}\u{0}');

    arktest.assertEQ(String.fromCharCode(0, MAX), '\u{0}\u{0}',
        'String built from char codes [0, MAX] must be \u{0}\u{0}');
    arktest.assertEQ(String.fromCharCode(-MAX, 0), '\u{0}\u{0}',
        'String built from char codes [-MAX, 0] must be \u{0}\u{0}');

    arktest.assertEQ(string.fromCharCode(-MAX, -MAX), '\u{0}\u{0}',
        'String built from char codes [-MAX, -MAX] must be \u{0}\u{0}');
    arktest.assertEQ(string.fromCharCode(MAX, MAX), '\u{0}\u{0}',
        'String built from char codes [MAX, MAX] must be \u{0}\u{0}');
}

function overflowSingleCasesTest() {
    const MAX = 65536
    arktest.assertEQ(string.fromCharCode(0), '\u{0}', 'String built from char code 0 must be 0');
    arktest.assertEQ(string.fromCharCode(MAX), '\u{0}', 'String built from char code MAX must be 0');
    arktest.assertEQ(string.fromCharCode(MAX - 1), '\u{ffff}',
        'String built from char code MAX - 1 must be 65535');
    arktest.assertEQ(string.fromCharCode(-MAX), '\u{0}',
        'String built from char code -MAX must be 0');
    arktest.assertEQ(string.fromCharCode(-(MAX - 1)), '\u{1}',
        'String built from char code -(MAX - 1) must be 1');
    arktest.assertEQ(string.fromCharCode(-(MAX + 1)), '\u{ffff}',
        'String built from char code -(MAX + 1) must be \u{ffff}');
}

function stringFromCpsSeqTest() {
    const cpsGolden = '\u{c}\u{7545}\u{ea7e}\u{15fb7}\u{1d4f0}\u{24a29}\u{2bf62}\u{3349b}\u{3a9d4}\u{41f0d}\u{49446}\u{5097f}\u{57eb8}\u{5f3f1}\u{6692a}\u{6de63}\u{7539c}\u{7c8d5}\u{83e0e}\u{8b347}\u{92880}\u{99db9}\u{a12f2}\u{a882b}\u{afd64}\u{b729d}\u{be7d6}\u{c5d0f}\u{cd248}\u{d4781}\u{dbcba}\u{e31f3}\u{ea72c}\u{f1c65}\u{f919e}\u{1006d7}\u{107c10}\u{10f149}';
    let cpsActual = '';
    for (let i: int = 12; i < 0x10FFFF; i += 30009) {
        if (Char.codeUnitsToEncode(i) == 1) {
            cpsActual += String.fromCharCode(i);
        } else {
            cpsActual += String.fromCharCode(Char.getHighSurrogate(i).toInt());
            cpsActual += String.fromCharCode(Char.getLowSurrogate(i).toInt());
        }
    }
    arktest.assertEQ(cpsActual, cpsGolden, 'cpsActual must be equal to cpsGolden');
}

function heapVerificationTest() {
    arktest.assertEQ(string.fromCharCode(127, 0), '\u{007f}\u{0}', 'String built from 127, NaN must be 127');
    arktest.assertEQ(string.fromCharCode(127, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(128, 0), '\u{0080}\u{0}', 'String built from 128, NaN must be 128');
    arktest.assertEQ(string.fromCharCode(128, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(-1, 0), '\u{ffff}\u{0}', 'String built from -1, NaN must be 65535');
    arktest.assertEQ(string.fromCharCode(-1, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(-100, 0, 0), 'ﾜ\u{0}\u{0}', "Error! Must be 'ﾜ\u{0}\u{0}'");
    arktest.assertEQ(string.fromCharCode(-100, 0, 0).length, 3);
    arktest.assertEQ(string.fromCharCode(-100, 0), 'ﾜ\u{0}', "Error! Must be 'ﾜ\u{0}'");
    arktest.assertEQ(string.fromCharCode(-100, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(-100, 0), 'ﾜ\u{0}', "Error! Must be 'ﾜ\u{0}'");
    arktest.assertEQ(string.fromCharCode(-100, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(65436, 0), 'ﾜ\u{0}', "Error! Must be 'ﾜ\u{0}'");
    arktest.assertEQ(string.fromCharCode(65436, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100, 0, 0), 'fﾜ\u{0}\u{0}', "Error! Must be 'fﾜ\u{0}\u{0}'");
    arktest.assertEQ(string.fromCharCode(0xffff0066, -100, 0, 0).length, 4);
    arktest.assertEQ(string.fromCharCode(65535, 0), '\u{ffff}\u{0}', 'String built from 65535, NaN must be 65535');
    arktest.assertEQ(string.fromCharCode(65535, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(65536, 0), '\u{0}\u{0}', 'String built from 65536, NaN must be 0');
    arktest.assertEQ(string.fromCharCode(65536, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(65435, 0), 'ﾛ\u{0}', "Error! Must be 'ﾛ\u{0}'");
    arktest.assertEQ(string.fromCharCode(65435, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(65437, 0), 'ﾝ\u{0}', "Error! Must be 'ﾝ\u{0}'");
    arktest.assertEQ(string.fromCharCode(65437, 0).length, 2);
    arktest.assertEQ(string.fromCharCode(126, 126, 125, 128, 0).length, 5);
    arktest.assertEQ(string.fromCharCode(126, 126, 125, 128, 127, 0).length, 6);
}

function stringFromCharVarTest() {
    let alpha: char = 0x3B1.toChar();
    let beta: char = 0x3B2.toChar();
    arktest.assertEQ(string.fromCharCode(alpha.toInt(), beta.toInt()), 'αβ', "Must be 'αβ'");
    arktest.assertEQ(string.fromCharCode(Char.toDouble(alpha), Char.toDouble(beta)), 'αβ', "Must be 'αβ'");
}

function stringFromSingleCharVarTest() {
    let beta: char = 0x3B2.toChar();
    arktest.assertEQ(string.fromCharCode(beta.toInt()), 'β', "Must be 'β'");
    arktest.assertEQ(string.fromCharCode(Char.toDouble(beta)), 'β', "Must be 'β'");
}

function stringFromCompositeSymbolTest() {
    arktest.assertEQ(string.fromCharCode(0xd83c, 0xdf03), '🌃');
    arktest.assertEQ(string.fromCharCode(0xd842, 0xdfb7), '𠮷');
}

function main(): int {
    let fromCharCodeTestsuite = new arktest.ArkTestsuite('intrinsics.string_from_char_code');
    fromCharCodeTestsuite.addTest('EmptyString', emptyStringTest);
    fromCharCodeTestsuite.addTest('CompressibleString', compressibleStringTest);
    fromCharCodeTestsuite.addTest('CompressibleSingleString', compressibleSingleStringTest);
    fromCharCodeTestsuite.addTest('IncompressibleString', incompressibleStringTest);
    fromCharCodeTestsuite.addTest('IncompressibleSingleString', incompressibleSingleStringTest);
    fromCharCodeTestsuite.addTest('FalsePositiveWrongSymbolFlag', falsePositiveWrongSymbolFlagTest);
    fromCharCodeTestsuite.addTest('FalsePositiveWrongSymbolFlagSingle', falsePositiveWrongSymbolFlagSingleTest);
    fromCharCodeTestsuite.addTest('NegativeCases', negativeCasesTest);
    fromCharCodeTestsuite.addTest('NegativeSingleCases', negativeSingleCasesTest);
    fromCharCodeTestsuite.addTest('SaturationCases', saturationCasesTest);
    fromCharCodeTestsuite.addTest('SaturationSingleCases', saturationSingleCasesTest);
    fromCharCodeTestsuite.addTest('OverflowCases', overflowCasesTest);
    fromCharCodeTestsuite.addTest('OverflowSingleCases', overflowSingleCasesTest);
    fromCharCodeTestsuite.addTest('StringFromCpsSeq', stringFromCpsSeqTest);
    fromCharCodeTestsuite.addTest('HeapVerification', heapVerificationTest);
    fromCharCodeTestsuite.addTest('StringFromCharVar', stringFromCharVarTest);
    fromCharCodeTestsuite.addTest('StringFromSingleCharVar', stringFromSingleCharVarTest);
    fromCharCodeTestsuite.addTest('StringFromCompositeSymbol', stringFromCompositeSymbolTest);
    return fromCharCodeTestsuite.run();
}

/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

const emptyString = '';
let nullString: String = new String(null);
let chArray: FixedArray<char> = [c'!', c'@', c'#', c'$', c'%'];

const b1: byte = Byte.MIN_VALUE as byte
const b2: byte = 0 as byte
const b3: byte = Byte.MAX_VALUE as byte
const s1: short = Short.MIN_VALUE as short
const s2: short = 0 as short
const s3: short = Short.MAX_VALUE as short
const i1: int = Int.MIN_VALUE as int
const i2: int = 0 as int
const i3: int = Int.MAX_VALUE as int
const l1: long = Long.MIN_VALUE
const l2: long = 0 as long
const l3: long = Long.MAX_VALUE
const f1: float = Float.MIN_VALUE as float
const f2: float = 0.0.toFloat()
const f3: float = Float.MAX_VALUE as float
const n1: number = Number.MIN_VALUE
const n2: number = 0.0
const n3: number = Number.MAX_VALUE

enum StringKind { Normal = 0, Empty = 1, Null = 2 };

// This is a sort of String factory, which is capable to
// generate Normal, Empty and Null strings.
function getString(kind: StringKind)
{
    if (kind == StringKind.Normal) {
        return 'This is a normal string';
    } else if (kind == StringKind.Empty) {
        return '';
    }
    arktest.assertEQ(kind, StringKind.Null);
    return nullString;
}

function main(): int {
    try {
        // =============== Test 1 ===============
        let sb1: StringBuilder = new StringBuilder(chArray);
        sb1.append(' ')
           .append('abcΣΨΩ0123456789')
           .append(' ')
           .append(true)
           .append(' ')
           .append(false)
           .append(' ')
           .append(57 as byte)
           .append(' ')
           .append(127 as byte)
           .append(' ')
           .append(32767 as short)
           .append(' ')
           .append(128934675 as int)
           .append(' ')
           .append(701234987654321 as long)
           .append(' ')
           .append(emptyString)
           .append(nullString);

        const golden1 = '!@#$% abcΣΨΩ0123456789 true false 57 127 32767 128934675 701234987654321 null'
        let actual1: String = sb1.toString();
        if (actual1 != golden1) {
            console.println('FAIL');
            console.println('Golden ' + '(len=' + golden1.getLength() + ')' + ': ' + golden1);
            console.println('Actual ' + '(len=' + actual1.getLength() + ')' + ': ' + actual1);
            return 1;
        }

        // =============== Test 2 ===============
        const cpsGolden = '\u{c}\u{7545}\u{ea7e}\u{15fb7}\u{1d4f0}\u{24a29}\u{2bf62}\u{3349b}\u{3a9d4}\u{41f0d}\u{49446}\u{5097f}\u{57eb8}\u{5f3f1}\u{6692a}\u{6de63}\u{7539c}\u{7c8d5}\u{83e0e}\u{8b347}\u{92880}\u{99db9}\u{a12f2}\u{a882b}\u{afd64}\u{b729d}\u{be7d6}\u{c5d0f}\u{cd248}\u{d4781}\u{dbcba}\u{e31f3}\u{ea72c}\u{f1c65}\u{f919e}\u{1006d7}\u{107c10}\u{10f149}'
        let cpsActual = '';
        for (let i = 12; i < 0x10FFFF; i += 30009) {
            if (Char.codeUnitsToEncode(i) == 1) {
                cpsActual += String.fromCharCode(i);
            } else {
                cpsActual += String.fromCharCode(Char.getHighSurrogate(i).toInt());
                cpsActual += String.fromCharCode(Char.getLowSurrogate(i).toInt());
            }
        }
        if (cpsActual != cpsGolden) {
            console.println('FAIL');
            return 2;
        }

        // =============== Test 3 ===============
        const embedExprGolden = 'A -128 B 0 C 127 D -32768 E 0 F 32767 G -2147483648 H 0 I 2147483647 J -9223372036854775808 K 0 L 9223372036854775807 M 1e-45 N 0 P 3.4028235e+38 Q 5e-324 R 0 S 1.7976931348623157e+308';
        const embedExprActual = `A ${b1} B ${b2} C ${b3} D ${s1} E ${s2} F ${s3} G ${i1} H ${i2} I ${i3} J ${l1} K ${l2} L ${l3} M ${f1} N ${f2} P ${f3} Q ${n1} R ${n2} S ${n3}`;
        if (embedExprActual != embedExprGolden) {
            console.println('FAIL');
            console.println('embedExprGolden ' + '(len=' + embedExprGolden.getLength() + ')' + ': ' + embedExprGolden);
            console.println('embedExprActual ' + '(len=' + embedExprActual.getLength() + ')' + ': ' + embedExprActual);
            return 3;
        }

        // =============== Test 4 ===============
        // An element of array may be changed after StringBuilder is constructed from the array.
        // So StringBuilder must add a copy of the array to the buffer.
        let arr4: FixedArray<char> = new FixedArray<char>(3)
        arr4[0] = c'a';
        arr4[1] = c'b';
        arr4[2] = c'c';
        let sb4: StringBuilder = new StringBuilder(arr4);
        arr4[1] = c'β';
        if (sb4.toString() != 'abc') {
            console.println('FAIL');
            return 4;
        }

        // =============== Test 5 ===============
        // Test Codegen implementation of StringBuilder.append(String).
        // The test try to cover all the cases encoded by Codegen::CreateStringBuilderAppendString.
        const golden5: String = 'abcΣΨΩ0123456789 | This is a normal string |  | null | null'
        // Local Null String to test IsNullPtr() branch of CreateStringBuilderAppendString.
        let nullString5: String = new String(null);
        let sb5: StringBuilder = new StringBuilder();
        sb5.append('abcΣΨΩ0123456789').append(' | ')
          .append(getString(StringKind.Normal)).append(' | ')
          .append(getString(StringKind.Empty)).append(' | ')
          .append(getString(StringKind.Null)).append(' | ')
          .append(nullString5);
        if (golden5 != sb5.toString()) {
            console.println('FAIL');
            console.println('Golden string: ' + golden5);
            console.println('Actual string: ' + sb5.toString());
            return 5;
        }

        // =============== Test 6 ===============
        // Test Codegen implementation of StringBuilder.append(String).
        // String.join() make use of StingBuilder.append(String)
        const golden6: String = '|pRef|AbCd|SUF|:|pRef|Xtt|SUF|:|pRef|G|SUF|';
        let actual6: String = String.join(['AbCd', 'Xtt', 'G'], ':', '|pRef|', '|SUF|');
        if (!actual6.equals(golden6 as Object)) {
            console.println('FAIL');
            console.println('Golden string: ' + golden6);
            console.println('Actual string: ' + actual6);
            return 6;
        }

    } catch(e) {
        console.println(e);
        console.println('FAIL');
        return 127;
    }
    return 0;
}

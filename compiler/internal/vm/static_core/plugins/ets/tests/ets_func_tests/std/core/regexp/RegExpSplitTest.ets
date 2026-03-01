/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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


function main(): int {
    const suite = new arktest.ArkTestsuite('RegExp.split')

    suite.addTest('SplitTest', splitTest)
    suite.addTest('SplitEmojiTest', splitEmojiTest)
    
    return suite.run()
}

function splitTest(): void {
    const str1 = 'A1B2'
    const regexp1 = new RegExp('\\p{L}', 'u')
    const result1 = str1.split(regexp1)
    arktest.assertEQ(result1[0], '');
    arktest.assertEQ(result1[1], '1');
    arktest.assertEQ(result1[2], '2');

    const str2 = 'hi😀bye'
    const regexp2 = new RegExp('\\p{Emoji}', 'u')
    const result2 = str2.split(regexp2)
    arktest.assertEQ(result2[0], 'hi');
    arktest.assertEQ(result2[1], 'bye');

    const str3 = 'A𠮷B𠮷C'
    const regexp3 = new RegExp('𠮷', 'u')
    const result3 = str3.split(regexp3)
    arktest.assertEQ(result3[0], 'A');
    arktest.assertEQ(result3[1], 'B');
    arktest.assertEQ(result3[2], 'C');

    const str4 = 'A😀C'
    const regexp4 = new RegExp('(?=\\p{Emoji})', 'u')
    const result4 = str4.split(regexp4)
    arktest.assertEQ(result4[0], 'A');
    arktest.assertEQ(result4[1], '😀C');
}

function splitEmojiTest(): void {
    const emoji1 = '\ud83d\ude0a'
    const regex1a = new RegExp('(?:)')
    const regex1b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji1.split(regex1a).toString(), '\ud83d,\ude0a');
    arktest.assertEQ(emoji1.split(regex1b).toString(), '😊');

    const emoji2 = '\ud83d\udc64'
    const regex2a = new RegExp('(?:)')
    const regex2b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji2.split(regex2a).toString(), '\ud83d,\udc64');
    arktest.assertEQ(emoji2.split(regex2b).toString(), '👤');

    const emoji3 = '\ud83d\udc6b'
    const regex3a = new RegExp('(?:)')
    const regex3b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji3.split(regex3a).toString(), '\ud83d,\udc6b');
    arktest.assertEQ(emoji3.split(regex3b).toString(), '👫');

    const emoji4 = '\u260e'
    const regex4a = new RegExp('(?:)')
    const regex4b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji4.split(regex4a).toString(), '\u260e');
    arktest.assertEQ(emoji4.split(regex4b).toString(), '☎');

    const emoji5 = '\ud83d\udc79'
    const regex5a = new RegExp('(?:)')
    const regex5b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji5.split(regex5a).toString(), '\ud83d,\udc79');
    arktest.assertEQ(emoji5.split(regex5b).toString(), '👹');

    const emoji6 = '\ud83d\udd5b'
    const regex6a = new RegExp('(?:)')
    const regex6b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji6.split(regex6a).toString(), '\ud83d,\udd5b');
    arktest.assertEQ(emoji6.split(regex6b).toString(), '🕛');

    const emoji7 = '\ud83c\udf4f'
    const regex7a = new RegExp('(?:)')
    const regex7b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji7.split(regex7a).toString(), '\ud83c,\udf4f');
    arktest.assertEQ(emoji7.split(regex7b).toString(), '🍏');

    const emoji8 = '\ud83d\udd03'
    const regex8a = new RegExp('(?:)')
    const regex8b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji8.split(regex8a).toString(), '\ud83d,\udd03');
    arktest.assertEQ(emoji8.split(regex8b).toString(), '🔃');

    const emoji9 = '\ud83c\udf41'
    const regex9a = new RegExp('(?:)')
    const regex9b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji9.split(regex9a).toString(), '\ud83c,\udf41');
    arktest.assertEQ(emoji9.split(regex9b).toString(), '🍁');

    const emoji10 = '\u2666'
    const regex10a = new RegExp('(?:)')
    const regex10b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji10.split(regex10a).toString(), '\u2666');
    arktest.assertEQ(emoji10.split(regex10b).toString(), '♦');

    const emoji11 = '\ud83e\udd3c\u200d\u2642\ufe0f'
    const regex11a = new RegExp('(?:)')
    const regex11b = new RegExp('(?:)', 'u')
    arktest.assertEQ(emoji11.split(regex11a).toString(), '\ud83e,\udd3c,\u200d,\u2642,\ufe0f');
    arktest.assertEQ(emoji11.split(regex11b).toString(), '🤼,‍,♂,️');
}
/**
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

// Ideally we'd like to avoid call (and test) anything but String.lastIndexOf(char).
// In particular the array of chars to avoid calling String.charAt(pos).

let empty = new string('');

let u8_line = new string('abcdefghijklmnopqrstuvwxyz');
let u8_chars = new Array<char>(c'a', c'b', c'c', c'd', c'e', c'f', c'g', c'h', c'i', c'j', c'k', c'l', c'm',
                               c'n', c'o', c'p', c'q', c'r', c's', c't', c'u', c'v', c'w', c'x', c'y', c'z');

let u8_line2 = new string('abcdefghabcdefgh');
let u8_chars2 = new Array<char>(c'a',c'b',c'c',c'd',c'e',c'f',c'g',c'h',
                                c'a',c'b',c'c',c'd',c'e',c'f',c'g',c'h');

let u8_line3 = new string('123412341234123412341234');
let u8_chars3 = new Array<char>(c'1',c'2',c'3',c'4',c'1',c'2',c'3',c'4',
                                c'1',c'2',c'3',c'4',c'1',c'2',c'3',c'4',
                                c'1',c'2',c'3',c'4',c'1',c'2',c'3',c'4')

let u8_line4 = new string('121112111211121112111211');
let u8_chars4 = new Array<char>(c'1',c'2',c'1',c'1',c'1',c'2',c'1',c'1',
                                c'1',c'2',c'1',c'1',c'1',c'2',c'1',c'1',
                                c'1',c'2',c'1',c'1',c'1',c'2',c'1',c'1')

let u8_line5 = new string('123443211234432112344321');
let u8_chars5 = new Array<char>(c'1',c'2',c'3',c'4',c'4',c'3',c'2',c'1',
                                c'1',c'2',c'3',c'4',c'4',c'3',c'2',c'1',
                                c'1',c'2',c'3',c'4',c'4',c'3',c'2',c'1')

let u8_tree: String = StringBuilder.concatStrings('123443211234', '432112344321');
let u8_tree_chars = new Array<char>(c'1',c'2',c'3',c'4',c'4',c'3',c'2',c'1',
                                    c'1',c'2',c'3',c'4',c'4',c'3',c'2',c'1',
                                    c'1',c'2',c'3',c'4',c'4',c'3',c'2',c'1')

let u8_sliced: String = '123443211234432112344321'.substr(4, 16);
let u8_sliced_chars = new Array<char>(c'4',c'3',c'2',c'1',c'1',c'2',c'3',c'4',
                                      c'4',c'3',c'2',c'1',c'1',c'2',c'3',c'4');


function u8_test(): int
{
    arktest.assertEQ(empty.lastIndexOf(c' '), -1);
    arktest.assertEQ(u8_line.lastIndexOf(c'零', -1), -1);
    arktest.assertEQ(u8_line.lastIndexOf(c'a', -1), -1);
    arktest.assertEQ(u8_line.lastIndexOf(c'a', u8_line.length), 0);
    arktest.assertEQ(u8_line.lastIndexOf(c'a', u8_line.length + 1), 0);

    arktest.assertEQ(u8_line.lastIndexOf(c'-'), -1);
    for (let i = 0; i < u8_line.length; ++i) {
        let c: char = u8_chars[i];
        let index = u8_line.lastIndexOf(c);
        arktest.assertEQ(index, i);
    }
    for (let i = 0; i < u8_line.length; ++i) {
        let c: char = u8_chars[i];
        let index = u8_line.lastIndexOf(c, i);
        arktest.assertEQ(index, i);
    }
    for (let i = 0; i < u8_line2.length; ++i) {
        let c: char = u8_chars2[i];
        let index = u8_line2.lastIndexOf(c);
        arktest.assertEQ(index, i%8 + 8);
    }
    for (let i = 0; i < u8_line3.length; ++i) {
        let c: char = u8_chars3[i];
        let index = u8_line3.lastIndexOf(c);
        if (c == c'1')
            arktest.assertEQ(index, 20);
        else if (c == c'2')
            arktest.assertEQ(index, 21);
        else if (c == c'3')
            arktest.assertEQ(index, 22);
        else if (c == c'4')
            arktest.assertEQ(index, 23);
        else
            arktest.assertTrue(false);
    }
    for (let i = 0; i < u8_line4.length; ++i) {
        let c: char = u8_chars4[i];
        let index = u8_line4.lastIndexOf(c);
        if (c == c'1')
            arktest.assertEQ(index, 23);
        else if (c == c'2')
            arktest.assertEQ(index, 21);
        else
            arktest.assertTrue(false);
    }
    for (let i = 0; i < u8_line5.length; ++i) {
        let c: char = u8_chars5[i];
        let index = u8_line5.lastIndexOf(c);
        if (c == c'1')
            arktest.assertEQ(index, 23);
        else if (c == c'2')
            arktest.assertEQ(index, 22);
        else if (c == c'3')
            arktest.assertEQ(index, 21);
        else if (c == c'4')
            arktest.assertEQ(index, 20);
        else
            arktest.assertTrue(false);
    }
    for (let i = 0; i < u8_tree.length; ++i) {
        let c: char = u8_tree_chars[i];
        let index = u8_tree.lastIndexOf(c);
        if (c == c'1')
            arktest.assertEQ(index, 23);
        else if (c == c'2')
            arktest.assertEQ(index, 22);
        else if (c == c'3')
            arktest.assertEQ(index, 21);
        else if (c == c'4')
            arktest.assertEQ(index, 20);
        else
            arktest.assertTrue(false);
    }
    for (let i = 0; i < u8_sliced.length; ++i) {
        let c: char = u8_sliced_chars[i];
        let index = u8_sliced.lastIndexOf(c);
        if (c == c'1')
            arktest.assertEQ(index, 12);
        else if (c == c'2')
            arktest.assertEQ(index, 13);
        else if (c == c'3')
            arktest.assertEQ(index, 14);
        else if (c == c'4')
            arktest.assertEQ(index, 15);
        else
            arktest.assertTrue(false);
    }
    return 0;
}

let u16_line = new string('零一二三四五六七八九音乐诗歌哲学物理');
let u16_chars = new Array<char>(c'零', c'一', c'二', c'三', c'四', c'五', c'六', c'七', c'八', c'九',
                                c'音', c'乐', c'诗', c'歌', c'哲', c'学', c'物', c'理');

let u16_line2 = new string('零一二三四五六七八九零一二三四五六七八九');
let u16_chars2 = new Array<char>(c'零', c'一', c'二', c'三', c'四', c'五', c'六', c'七', c'八', c'九',
                                 c'零', c'一', c'二', c'三', c'四', c'五', c'六', c'七', c'八', c'九');

let u16_line3 = new string('一二三四一二三四一二三四一二三四一二三四一二三四');
let u16_chars3 = new Array<char>(c'一', c'二', c'三', c'四', c'一', c'二', c'三', c'四',
                                 c'一', c'二', c'三', c'四', c'一', c'二', c'三', c'四',
                                 c'一', c'二', c'三', c'四', c'一', c'二', c'三', c'四');

let u16_line4 = new string('一二一一一二一一一二一一一二一一一二一一一二一一');
let u16_chars4 = new Array<char>(c'一', c'二', c'一', c'一', c'一', c'二', c'一', c'一',
                                 c'一', c'二', c'一', c'一', c'一', c'二', c'一', c'一',
                                 c'一', c'二', c'一', c'一', c'一', c'二', c'一', c'一');

let u16_line5 = new string('一二三四四三二一一二三四四三二一一二三四四三二一');
let u16_chars5 = new Array<char>(c'一', c'二', c'三', c'四', c'四', c'三', c'二', c'一',
                                 c'一', c'二', c'三', c'四', c'四', c'三', c'二', c'一',
                                 c'一', c'二', c'三', c'四', c'四', c'三', c'二', c'一');

let u16_tree: String = StringBuilder.concatStrings('零一二三四五六七八九', '零一二三四五六七八九');
let u16_tree_chars = new Array<char>(c'零', c'一', c'二', c'三', c'四', c'五', c'六', c'七', c'八', c'九',
                                     c'零', c'一', c'二', c'三', c'四', c'五', c'六', c'七', c'八', c'九');

let u16_sliced: String = '零一二三四五六七八九音乐诗歌哲学物理'.substr(0, 10);
let u16_sliced_chars = new Array<char>(c'零', c'一', c'二', c'三', c'四', c'五', c'六', c'七', c'八', c'九');


function u16_test(): int
{
    arktest.assertEQ(u16_line.lastIndexOf(c'-'), -1);
    arktest.assertEQ(u16_line.lastIndexOf(c'零', -1), -1);
    arktest.assertEQ(u16_line.lastIndexOf(c'零', u16_line.length), 0);
    arktest.assertEQ(u16_line.lastIndexOf(c'零', u16_line.length + 1), 0);

    for (let i = 0; i < u16_line.length; ++i) {
        let c: char = u16_chars[i];
        let index = u16_line.lastIndexOf(c);
        arktest.assertEQ(index, i);
    }
    for (let i = 0; i < u16_line.length; ++i) {
        let c: char = u16_chars[i];
        let index = u16_line.lastIndexOf(c, i);
        arktest.assertEQ(index, i);
    }
    for (let i = 0; i < u16_line2.length; ++i) {
        let c: char = u16_chars2[i];
        let index = u16_line2.lastIndexOf(c);
        arktest.assertEQ(index, i%10 + 10);
    }
    for (let i = 0; i < u16_line3.length; ++i) {
        let c: char = u16_chars3[i];
        let index = u16_line3.lastIndexOf(c);
        if (c == c'一')
            arktest.assertEQ(index, 20);
        else if (c == c'二')
            arktest.assertEQ(index, 21);
        else if (c == c'三')
            arktest.assertEQ(index, 22);
        else if (c == c'四')
            arktest.assertEQ(index, 23);
        else
            arktest.assertTrue(false);
    }
    for (let i = 0; i < u16_line4.length; ++i) {
        let c: char = u16_chars4[i];
        let index = u16_line4.lastIndexOf(c);
        if (c == c'一')
            arktest.assertEQ(index, 23);
        else if (c == c'二')
            arktest.assertEQ(index, 21);
        else
            arktest.assertTrue(false);
    }
    for (let i = 0; i < u16_line5.length; ++i) {
        let c: char = u16_chars5[i];
        let index = u16_line5.lastIndexOf(c);
        if (c == c'一')
            arktest.assertEQ(index, 23);
        else if (c == c'二')
            arktest.assertEQ(index, 22);
        else if (c == c'三')
            arktest.assertEQ(index, 21);
        else if (c == c'四')
            arktest.assertEQ(index, 20);
        else
            arktest.assertTrue(false);
    }
    for (let i = 0; i < u16_tree.length; ++i) {
        let c: char = u16_tree_chars[i];
        let index = u16_tree.lastIndexOf(c);
        arktest.assertEQ(index, i%10 + 10);
    }
    for (let i = 0; i < u16_sliced.length; ++i) {
        let c: char = u16_sliced_chars[i];
        let index = u16_sliced.lastIndexOf(c);
        arktest.assertEQ(index, i%10);
    }
    return 0;
}

function main() {
    u8_test();
    u16_test();
}

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


function main(): int {
    let suite = new arktest.ArkTestsuite('StringIsEmpty');
    suite.addTest('test LineString', isEmptyLineTest);
    suite.addTest('test SlicedString', isEmptySlicedTest);
    suite.addTest('test TreeString', isEmptyTreeTest);
    return suite.run();
}

function isEmptyLineTest() {
    let str: String = '';
    let result: boolean = str.isEmpty();
    arktest.assertEQ(result, true);
    str = 'abcde';
    result = str.isEmpty();
    arktest.assertEQ(result, false);
}

function isEmptySlicedTest() {
    let strutf8: String = 'abcdefghijklmnopqrstuvwxyz';
    let slicedStr = strutf8.substring(1, 25); // slicedStr is SliceString
    arktest.assertEQ(slicedStr.length, 24);
    arktest.assertEQ(slicedStr.isEmpty(), false);
    let strutf16: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let slicedStrutf16 = strutf8.substring(1, 25); // slicedStr is SliceString
    arktest.assertEQ(slicedStrutf16.length, 24);
    arktest.assertEQ(slicedStrutf16.isEmpty(), false);
}

function isEmptyTreeTest() {
    let str1: String = 'abcdefghijklmn';
    let str2: String = 'opqrstuvwxyz';
    let treeStr = str1.concat(str2); // treeStr is TreeString
    arktest.assertEQ(treeStr.isEmpty(), false);
    let str1Utf16: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛ';
    let str2Utf16: String = 'лМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя';
    let treeStrUtf16 = str1Utf16.concat(str2Utf16); // treeStr is TreeString
    arktest.assertEQ(treeStrUtf16.isEmpty(), false);
}

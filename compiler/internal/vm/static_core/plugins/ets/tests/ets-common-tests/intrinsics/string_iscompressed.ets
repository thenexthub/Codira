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

const RusChars: String = 'АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя'; // 16-bit
const EngChars: String = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz';               //  8-bit

function testIsCompressed(): void {
    arktest.assertFalse(RusChars.isCompressed());
    arktest.assertTrue(EngChars.isCompressed());

    let utf16Temp = '𐍈𐌰𐌱𐌲𐌳𐌴𐌵𐌶𐌷𐌸𐌹𐌺𐌻𐌼𐌽𐌾𐌿';
    let treeUtf16Chars = RusChars.concat(utf16Temp);
    arktest.assertFalse(treeUtf16Chars.isCompressed());
    treeUtf16Chars = treeUtf16Chars.concat(EngChars);
    arktest.assertFalse(treeUtf16Chars.isCompressed());
    let slicedUtf16Chars = treeUtf16Chars.substring(2, 19);
    arktest.assertFalse(slicedUtf16Chars.isCompressed());
    slicedUtf16Chars = RusChars.substring(2, 22);
    arktest.assertFalse(slicedUtf16Chars.isCompressed());

    let utf8Temp = '1234567890';
    let treeUtf8Chars = EngChars.concat(utf8Temp);
    arktest.assertTrue(treeUtf8Chars.isCompressed());
    let slicedUtf8Chars = treeUtf8Chars.substring(2, 10);
    arktest.assertTrue(slicedUtf8Chars.isCompressed());
    slicedUtf8Chars = EngChars.substring(2, 10);
    arktest.assertTrue(slicedUtf8Chars.isCompressed());
}

function main() {
    let suite = new arktest.ArkTestsuite('StringIsCompressed');
    suite.addTest('testIsCompressed', testIsCompressed);
    return suite.run();
}

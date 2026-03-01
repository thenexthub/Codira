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


function testFromCodePoint() {
    arktest.assertTrue(String.fromCodePoint(42) === '*'); // "*"
    arktest.assertTrue(String.fromCodePoint(65, 90) === 'AZ'); // "AZ"
    arktest.assertTrue(String.fromCodePoint(0x404) === 'Є'); // "\u0404" === "Є"
    arktest.assertTrue(String.fromCodePoint(0x2f804) === '\uD87E\uDC04'); // "\uD87E\uDC04"
    arktest.assertTrue(String.fromCodePoint(194564) === '\uD87E\uDC04'); // "\uD87E\uDC04"
    arktest.assertTrue(String.fromCodePoint(0x1d306, 0x61, 0x1d307) === '\uD834\uDF06a\uD834\uDF07'); // "\uD834\uDF06a\uD834\uDF07"
    arktest.assertTrue(String.fromCodePoint(65) === 'A');
    arktest.assertTrue(String.fromCodePoint(0x1F600) === '😀');
    arktest.assertTrue(String.fromCodePoint(0) === '\u0000'); // min value
    arktest.assertTrue(String.fromCodePoint(0x10FFFF) === '\u{10FFFF}'); // max value
    arktest.assertTrue(String.fromCodePoint(65, 66) === 'AB');
    arktest.assertTrue(String.fromCodePoint(0x1F600, 0x1F601) === '😀😁');

    // negative number
    arktest.expectError(() => { String.fromCodePoint(-1) });
    // Bigger than 0x10FFFF
    arktest.expectError(() => { String.fromCodePoint(0x10FFFF + 1) });
}

function main(): void {
    let suite = new arktest.ArkTestsuite('StringFromCodePoint');
    suite.addTest('testFromCodePoint', testFromCodePoint);
    suite.run();
}

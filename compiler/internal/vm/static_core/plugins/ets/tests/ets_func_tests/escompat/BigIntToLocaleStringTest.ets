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

const val = 123456789123456789n

function main(): int {
    const suite = new arktest.ArkTestsuite('BigInt ToLocalString Tests');
    suite.addTest('test toLocalStringTest1', toLocalStringTest1);
    suite.addTest('test toLocalStringTest2', toLocalStringTest2);
    suite.addTest('test toLocalStringTest3', toLocalStringTest3);
    suite.addTest('test toLocalStringTest4', toLocalStringTest4);
    suite.addTest('test toLocalStringTest5', toLocalStringTest5);
    suite.addTest('test testUseGrouping', testUseGrouping);
    suite.addTest('test testLocaleMatcher', testLocaleMatcher);
    suite.addTest('test testNumberingSystem', testNumberingSystem);
    suite.addTest('test testNotationCompact', testNotationCompact);
    suite.addTest('test testNotationScientific', testNotationScientific);
    suite.addTest('test testNotationEngineering', testNotationEngineering);
    suite.addTest('test testMinimumIntegerDigits', testMinimumIntegerDigits);
    suite.addTest('test testCurrency', testCurrency);
    suite.addTest('test testUnit', testUnit);
    suite.addTest('test testMinimumFractionDigits', testMinimumFractionDigits);
    suite.addTest('test testMaximumFractionDigits', testMaximumFractionDigits);
    suite.addTest('test testMinimumSigniFractionDigits', testMinimumSigniFractionDigits);
    suite.addTest('test testMaximumSignFractionDigits', testMaximumSignFractionDigits);
    return suite.run();
}

function toLocalStringTest1(): void {
    arktest.assertEQ('1,23,45,67,89,12,34,56,789', val.toLocaleString('en-IN'))
}

function toLocalStringTest2(): void {
    arktest.assertEQ('123,456,789,123,456,789', val.toLocaleString('en-US'))
}

function toLocalStringTest3(): void {
    arktest.assertEQ('123.456.789.123.456.789', val.toLocaleString('de-DE'))
}

function toLocalStringTest4(): void {
    arktest.assertEQ('123456789123456789', val.toLocaleString('en-US', { useGrouping: false }))
}

function toLocalStringTest5(): void {
    arktest.assertEQ('١٢٣٬٤٥٦٬٧٨٩٬١٢٣٬٤٥٦٬٧٨٩', val.toLocaleString('ar-EG', { useGrouping: true, numberingSystem: 'arab' }))
}

function testUseGrouping(): void {
    arktest.assertEQ('123456789123456789', val.toLocaleString('en-IN', { useGrouping: false }));
}

function testLocaleMatcher(): void {
    arktest.assertEQ('1,23,45,67,89,12,34,56,789', val.toLocaleString('en-IN', { localeMatcher: 'lookup' }));
}

function testNumberingSystem(): void {
    arktest.assertEQ('١٢٣٬٤٥٦٬٧٨٩٬١٢٣٬٤٥٦٬٧٨٩', val.toLocaleString('ar-EG', { numberingSystem: 'arab' }));
}

function testNotationCompact(): void {
    let num = 12345n
    arktest.assertEQ('12K', num.toLocaleString('en', { notation: 'compact', compactDisplay: 'short' }));
}

function testNotationScientific(): void {
    arktest.assertEQ('1.235E17', val.toLocaleString('en', { notation: 'scientific' }));
}

function testNotationEngineering(): void {
    arktest.assertEQ('123.457E15', val.toLocaleString('en', { notation: 'engineering' }));
}

function testMinimumIntegerDigits(): void {
    arktest.assertEQ('000123456789123456789', val.toLocaleString('en', { minimumIntegerDigits: 21, useGrouping: false }));
}

function testCurrency(): void {
    arktest.assertEQ('$123,456,789,123,456,789.00', val.toLocaleString('en', { style: 'currency', currency: 'USD' }));
}

function testUnit(): void {
    arktest.assertEQ('123,456,789,123,456,789 km', val.toLocaleString('en', { style: 'unit', unit: 'kilometer' }));
}

function testMinimumFractionDigits(): void {
    arktest.assertEQ('123,456,789,123,456,789.00', val.toLocaleString('en', { minimumFractionDigits: 2 }));
}

function testMaximumFractionDigits(): void {
    arktest.assertEQ('123,456,789,123,456,789', val.toLocaleString('en', { maximumFractionDigits: 2 }));
}

function testMinimumSigniFractionDigits(): void {
    arktest.assertEQ('123,456,789,123,456,789', val.toLocaleString('en', { minimumSignificantDigits: 3 }));
}

function testMaximumSignFractionDigits(): void {
    arktest.assertEQ('123,000,000,000,000,000', val.toLocaleString('en', { maximumSignificantDigits: 3 }));
}
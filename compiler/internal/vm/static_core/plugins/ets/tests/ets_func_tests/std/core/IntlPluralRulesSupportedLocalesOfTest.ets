/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

function main() {
    let myTestsuite = new arktest.ArkTestsuite('myTestsuite');
    myTestsuite.addTest('supportedLocalesOfTest01', supportedLocalesOfTest01);
    myTestsuite.addTest('supportedLocalesOfTest02', supportedLocalesOfTest02);
    myTestsuite.addTest('supportedLocalesOfTest03', supportedLocalesOfTest03);
    myTestsuite.addTest('supportedLocalesOfTest04', supportedLocalesOfTest04);
    myTestsuite.addTest('supportedLocalesOfTest05', supportedLocalesOfTest05);
    myTestsuite.addTest('supportedLocalesOfTest06', supportedLocalesOfTest06);
    myTestsuite.addTest('supportedLocalesOfTest07', supportedLocalesOfTest07);
    myTestsuite.addTest('supportedLocalesOfTest08', supportedLocalesOfTest08);
    myTestsuite.addTest('supportedLocalesOfTest09', supportedLocalesOfTest09);
    myTestsuite.addTest('supportedLocalesOfTest10', supportedLocalesOfTest10);
    myTestsuite.run();
}

function supportedLocalesOfTest01() {
    const locales: FixedArray<string> = ['ban']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 0)
}

function supportedLocalesOfTest02() {
    const locales: FixedArray<string> = ['ban', 'id-u-co-pinyin', 'de-ID', 'zh-u-co-pinyin', 'en-US']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 4)
    arktest.assertEQ(result[0], 'id-u-co-pinyin')
    arktest.assertEQ(result[1], 'de-ID')
    arktest.assertEQ(result[2], 'zh-u-co-pinyin')
    arktest.assertEQ(result[3], 'en-US')
}

function supportedLocalesOfTest03() {
    const locales: FixedArray<string> = ['ban', 'id-u-co-pinyin', 'de-iD', 'zh-u-co-pinyin', 'en-US']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 4)
    arktest.assertEQ(result[0], 'id-u-co-pinyin')
    arktest.assertEQ(result[1], 'de-ID')
    arktest.assertEQ(result[2], 'zh-u-co-pinyin')
    arktest.assertEQ(result[3], 'en-US')
}

function supportedLocalesOfTest04() {
    const locales: FixedArray<string> = ['ban', 'id-u-co-pinyin', 'de-ID', 'De-id', 'zh-u-co-pinyin', 'en-US', 'de-ID', 'De-id']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 4)
    arktest.assertEQ(result[0], 'id-u-co-pinyin')
    arktest.assertEQ(result[1], 'de-ID')
    arktest.assertEQ(result[2], 'zh-u-co-pinyin')
    arktest.assertEQ(result[3], 'en-US')
}

function supportedLocalesOfTest05() {
    const locales: Array<string> = ['ban']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 0)
}

function supportedLocalesOfTest06() {
    const locales: FixedArray<string> = ['']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    try {
        const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)
    } catch(error) {
        arktest.assertEQ(error.message, 'RangeError: invalid locale')
    }
}

function supportedLocalesOfTest07() {
    const locales: Array<string> = ['ban', 'id-u-co-pinyin', 'de-ID', 'zh-u-co-pinyin', 'en-US']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 4)
    arktest.assertEQ(result[0], 'id-u-co-pinyin')
    arktest.assertEQ(result[1], 'de-ID')
    arktest.assertEQ(result[2], 'zh-u-co-pinyin')
    arktest.assertEQ(result[3], 'en-US')
}

function supportedLocalesOfTest08() {
    const locales: Array<string> = ['']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    try {
        const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)
    } catch(error) {
        arktest.assertEQ(error.message, 'RangeError: invalid locale')
    }
}

function supportedLocalesOfTest09() {
    const locales: FixedArray<string> = ['en-US']
    const options = {localeMatcher:'lookup'} as Intl.PluralRulesOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 1)
}

function supportedLocalesOfTest10() {
    const locales: FixedArray<string> = ['en-US']
    const options = {localeMatcher:'lookup'} as Intl.SupportedLocalesOfOptions
    const result: Array<string> = Intl.PluralRules.supportedLocalesOf(locales, options)

    arktest.assertEQ(result.length, 1)
}


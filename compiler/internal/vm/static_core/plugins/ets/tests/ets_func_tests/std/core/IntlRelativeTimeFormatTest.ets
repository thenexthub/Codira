/**
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

const fakeLocale = 'er-or'

function main(): int  {
    const suite = new arktest.ArkTestsuite('Intl.RelativeTimeFormat format');
    suite.addTest('testFormat', testFormat);
    suite.addTest('testFormatToParts', testFormatToParts);
    suite.addTest('testResolvedOptions', testResolvedOptions);
    suite.addTest('testSelectWithOptions', testSelectWithOptions);
    suite.addTest('testSupportedLocalesOf', testSupportedLocalesOf);
    return suite.run();
}

function convertAllWhiteSpacesDetailed(text: string): string {
    let result = text;
    
    result = result.replace('\u0009', ' ');
    
    result = result.replace('\u000B', ' ');
    
    result = result.replace('\u000C', ' ');
    
    result = result.replace('\u00A0', ' ');
    
    result = result.replace('\uFEFF', ' ');
    
    return result;
}

function runFormatTest(locale: Intl.UnicodeBCP47LocaleIdentifier,
                       value: number,
                       unit: Intl.RelativeTimeFormatUnit,
                       expected: string,
                       options?: Intl.RelativeTimeFormatOptions) {
    const format = new Intl.RelativeTimeFormat(locale, options)
    let result = format.format(value, unit)
    result = convertAllWhiteSpacesDetailed(result);
    arktest.assertEQ(result, expected)
}

function testFormat() {
    runFormatTest('en', 1, 'day', 'tomorrow', { numeric: 'auto' })
    runFormatTest('en', -1, 'day', 'yesterday', { numeric: 'auto' })
    runFormatTest('ru', 2, 'days', 'послезавтра', { numeric: 'auto' })
    runFormatTest('ru', 0, 'day', 'через 0 дней', { numeric: 'always' })
    runFormatTest('fr', 5, 'month', 'dans 5 mois', { style: 'long' })
    runFormatTest('fr', -5, 'month', 'il y a 5 mois', { style: 'long' })
    runFormatTest('ja', 10, 'second', '10 秒後', { style: 'long' })
    runFormatTest('en', -2, 'week', '2 wk. ago', { style: 'short' })
    runFormatTest('en', 3, 'quarter', 'in 3 qtrs.', { style: 'narrow' })
    runFormatTest('en', 1, 'day', 'in 1 day', { numeric: 'always' })
}

function runFormatToPartsTest(locale: string, values: FixedArray<number>, unit: string, expected: FixedArray<string>,
                              options: Intl.RelativeTimeFormatOptions) {
    const format = new Intl.RelativeTimeFormat(locale, options);
    for (let j = 0; j < values.length; j++) {
        let result = format.formatToParts(values[j], unit as Intl.RelativeTimeFormatUnit);
        let str: string = convertAllWhiteSpacesDetailed(result[0].value);
        arktest.assertEQ(str, expected[j]);
    }
}

function formatToPartsForSencodTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'long', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['second', 'seconds'];
    const values: FixedArray<number> = [0, 1, 100, -1, -100];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['0秒钟后', '1秒钟后', '100秒钟后', '1秒钟前', '100秒钟前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['через 0 секунд', 'через 1 секунду', 'через 100 секунд', '1 секунду назад', '100 секунд назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['in 0 seconds', 'in 1 second', 'in 100 seconds', '1 second ago', '100 seconds ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['dans 0 seconde', 'dans 1 seconde', 'dans 100 secondes', 'il y a 1 seconde', 'il y a 100 secondes'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForMinuteTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'short', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['minute', 'minutes'];
    const values: FixedArray<number> = [0, 1, 100, -1, -100];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['0分钟后', '1分钟后', '100分钟后', '1分钟前', '100分钟前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['через 0 мин.', 'через 1 мин.', 'через 100 мин.', '1 мин. назад', '100 мин. назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['in 0 min.', 'in 1 min.', 'in 100 min.', '1 min. ago', '100 min. ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['dans 0 min', 'dans 1 min', 'dans 100 min', 'il y a 1 min', 'il y a 100 min'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForHourTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'narrow', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['hour', 'hours'];
    const values: FixedArray<number> = [0, 1, 25, -1, -25];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['0小时后', '1小时后', '25小时后', '1小时前', '25小时前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['+0 ч', '+1 ч', '+25 ч', '-1 ч', '-25 ч'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['in 0 hr.', 'in 1 hr.', 'in 25 hr.', '1 hr. ago', '25 hr. ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['+0 h', '+1 h', '+25 h', '-1 h', '-25 h'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForDayTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'always', style: 'long', localeMatcher: 'lookup'};
    const units: FixedArray<string> = ['day', 'days'];
    const values: FixedArray<number> = [0, 1, 2, 3, -1, -2, -3];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['0天后', '1天后', '2天后', '3天后', '1天前', '2天前', '3天前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['через 0 дней', 'через 1 день', 'через 2 дня', 'через 3 дня', '1 день назад', '2 дня назад', '3 дня назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['in 0 days', 'in 1 day', 'in 2 days', 'in 3 days', '1 day ago', '2 days ago', '3 days ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['dans 0 jour', 'dans 1 jour', 'dans 2 jours', 'dans 3 jours', 'il y a 1 jour', 'il y a 2 jours', 'il y a 3 jours'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForWeekTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'long', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['week', 'weeks'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['本周', '下周', '2周后', '上周', '2周前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['на этой неделе', 'на следующей неделе', 'через 2 недели', 'на прошлой неделе', '2 недели назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this week', 'next week', 'in 2 weeks', 'last week', '2 weeks ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['cette semaine', 'la semaine prochaine', 'dans 2 semaines', 'la semaine dernière', 'il y a 2 semaines'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForMonthTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'short', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['month', 'months'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['本月', '下个月', '2个月后', '上个月', '2个月前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['в этом мес.', 'в следующем мес.', 'через 2 мес.', 'в прошлом мес.', '2 мес. назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this mo.', 'next mo.', 'in 2 mo.', 'last mo.', '2 mo. ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['ce mois-ci', 'le mois prochain', 'dans 2 m.', 'le mois dernier', 'il y a 2 m.'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForQuarterTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'narrow', localeMatcher: 'best fit'};
    const units: FixedArray<string> = ['quarter', 'quarters'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['本季度', '下季度', '2个季度后', '上季度', '2个季度前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['тек. кв.', 'след. кв.', '+2 кв.', 'посл. кв.', '-2 кв.'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this qtr.', 'next qtr.', 'in 2 qtrs.', 'last qtr.', '2 qtrs. ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['ce trimestre', 'le trimestre prochain', '+2 trim.', 'le trimestre dernier', '-2 trim.'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function formatToPartsForYearTest() {
    const options: Intl.RelativeTimeFormatOptions = {numeric: 'auto', style: 'long', localeMatcher: 'lookup'};
    const units: FixedArray<string> = ['year', 'years'];
    const values: FixedArray<number> = [0, 1, 2, -1, -2];
    for (let i = 0; i < units.length; i++) {
        let expected: FixedArray<string> = ['今年', '明年', '2年后', '去年', '2年前'];
        runFormatToPartsTest('zh', values, units[i], expected, options);

        expected = ['в этом году', 'в следующем году', 'через 2 года', 'в прошлом году', '2 года назад'];
        runFormatToPartsTest('ru', values, units[i], expected, options);

        expected = ['this year', 'next year', 'in 2 years', 'last year', '2 years ago'];
        runFormatToPartsTest('en', values, units[i], expected, options);

        expected = ['cette année', 'l’année prochaine', 'dans 2 ans', 'l’année dernière', 'il y a 2 ans'];
        runFormatToPartsTest('fr', values, units[i], expected, options);
    }
}

function testFormatToParts() {
    formatToPartsForSencodTest();
    formatToPartsForMinuteTest();
    formatToPartsForHourTest();
    formatToPartsForDayTest();
    formatToPartsForWeekTest();
    formatToPartsForMonthTest();
    formatToPartsForQuarterTest();
    formatToPartsForYearTest();
    testRelativeFormat();
}

function testResolvedOptions() {
    {
        const format = new Intl.RelativeTimeFormat('en')
        const result = format.resolvedOptions()
        console.log(result)
        arktest.assertEQ(result.style, 'long')
        arktest.assertEQ(result.numeric, 'always')
        arktest.assertEQ(result.numberingSystem, 'latn')
    }
    {
        const format = new Intl.RelativeTimeFormat('en', { style: 'short' } as Intl.RelativeTimeFormatOptions)
        const result = format.resolvedOptions()
        arktest.assertEQ(result.style, 'short')
    }
    {
        const format = new Intl.RelativeTimeFormat('en', { numeric: 'always' } as Intl.RelativeTimeFormatOptions)
        const result = format.resolvedOptions()
        arktest.assertEQ(result.numeric, 'always')
    }
}

function runTestWithOptions(locale: Intl.UnicodeBCP47LocaleIdentifier,
                            value: number,
                            unit: Intl.RelativeTimeFormatUnit,
                            options: Intl.RelativeTimeFormatOptions,
                            expected: string) {
    const format = new Intl.RelativeTimeFormat(locale, options)
    const result = format.format(value, unit)
    let str: string = convertAllWhiteSpacesDetailed(result);
    arktest.assertEQ(str, expected)
}

function testSelectWithOptions() {
    runTestWithOptions('en', 1, 'day', { numeric: 'auto' }, 'tomorrow')
    runTestWithOptions('en', -1, 'hour', { numeric: 'auto' }, '1 hour ago')
    runTestWithOptions('en', 0, 'minute', { numeric: 'always' }, 'in 0 minutes')
    runTestWithOptions('ru', 3, 'month', { style: 'long' }, 'через 3 месяца')
    runTestWithOptions('fr', 7, 'day', { style: 'short' }, 'dans 7 j')
    runTestWithOptions('zh', -7, 'day', { style: 'short' }, '7天前')
}

function runSupportedLocalesStringTest(locales: string,
                                       options: Intl.RelativeTimeFormatOptions | undefined,
                                       expected: string[]) {
    const result = Intl.RelativeTimeFormat.supportedLocalesOf(locales, options)
    arktest.assertEQ(result.length, expected.length)
    for (let i: int = 0; i < result.length; i++) {
        const isEq = result[i].includes(expected[i])
        arktest.assertTrue(isEq);
    }
}

function runSupportedLocalesArrayTest(locales: string[],
                                      options: Intl.RelativeTimeFormatOptions | undefined,
                                      expected: string[]) {
    const result = Intl.RelativeTimeFormat.supportedLocalesOf(locales, options)
    arktest.assertEQ(result.length, expected.length)
    for (let i: int = 0; i < result.length; i++) {
        const isEq = result[i].includes(expected[i])
        arktest.assertTrue(isEq);
    }
}

function testSupportedLocalesOf() {
    runSupportedLocalesStringTest(
        'en',
        undefined,
        Intl.intlBestFitLocales('en')
    )
    runSupportedLocalesStringTest(
        fakeLocale,
        undefined,
        Intl.intlBestFitLocales(fakeLocale)
    )
    runSupportedLocalesStringTest(
        'en',
        { localeMatcher: 'lookup' } as Intl.RelativeTimeFormatOptions,
        Intl.intlLookUpLocales('en')
    )
    runSupportedLocalesArrayTest(
        ['fr', 'ru'],
        undefined,
        Intl.intlBestFitLocales(['fr', 'ru'])
    )
    runSupportedLocalesArrayTest(
        ['ru-RU', 'zh-CN', 'en-US'],
        undefined,
        Intl.intlBestFitLocales(['ru-RU', 'zh-CN', 'en-US'])
    )
    runSupportedLocalesArrayTest(
        ['ru-RU', 'zh-CN', 'en-US'],
        { localeMatcher: 'best fit' } as Intl.RelativeTimeFormatOptions,
        Intl.intlBestFitLocales(['ru-RU', 'zh-CN', 'en-US'])
    )

    runSupportedLocalesArrayTest(
        ['ru-RU', 'en-US'],
        { localeMatcher: 'lookup' } as Intl.RelativeTimeFormatOptions,
        Intl.intlLookUpLocales(['ru-RU', 'en-US'])
    )
}

function testRelativeFormat() {
    let relativetimefmt = new Intl.RelativeTimeFormat('en', { style: 'narrow'});
    let value = relativetimefmt.format(100, 'second');
    arktest.assertEQ(value, 'in 100 sec.');

    relativetimefmt = new Intl.RelativeTimeFormat('en', { style: 'narrow'});
    value = relativetimefmt.format(-1, 'day');
    arktest.assertEQ(value, '1 day ago');

    relativetimefmt = new Intl.RelativeTimeFormat('zh', { style: 'narrow'});
    value = relativetimefmt.format(-1, 'day');
    arktest.assertEQ(value, '1天前');

    relativetimefmt = new Intl.RelativeTimeFormat('en', { style: 'narrow'});
    value = relativetimefmt.format(2, 'day');
    arktest.assertEQ(value, 'in 2 days');

    relativetimefmt = new Intl.RelativeTimeFormat('zh', { style: 'narrow'});
    value = relativetimefmt.format(2, 'day');
    arktest.assertEQ(value, '2天后');

    relativetimefmt = new Intl.RelativeTimeFormat('en', { numeric: 'auto', style: 'short'});
    value = relativetimefmt.format(-1, 'quarter');
    arktest.assertEQ(value, 'last qtr.');
    arktest.assertEQ(relativetimefmt.resolvedOptions().numeric, 'auto');
    arktest.assertEQ(relativetimefmt.resolvedOptions().style, 'short');

    relativetimefmt = new Intl.RelativeTimeFormat('en', { numeric: 'auto', style: 'short'});
    value = relativetimefmt.format(1, 'quarter');
    arktest.assertEQ(value, 'next qtr.');
    arktest.assertEQ(relativetimefmt.resolvedOptions().numeric, 'auto');
    arktest.assertEQ(relativetimefmt.resolvedOptions().style, 'short');

    relativetimefmt = new Intl.RelativeTimeFormat('en', { numeric: 'auto', style: 'narrow'});
    value = relativetimefmt.format(1, 'quarter');
    arktest.assertEQ(value, 'next qtr.');
    arktest.assertEQ(relativetimefmt.resolvedOptions().numeric, 'auto');
    arktest.assertEQ(relativetimefmt.resolvedOptions().style, 'narrow');

    relativetimefmt = new Intl.RelativeTimeFormat('en', { numeric: 'always', style: 'narrow'});
    value = relativetimefmt.format(1, 'quarter');
    arktest.assertEQ(value, 'in 1 qtr.');
    arktest.assertEQ(relativetimefmt.resolvedOptions().numeric, 'always');
    arktest.assertEQ(relativetimefmt.resolvedOptions().style, 'narrow');

    relativetimefmt = new Intl.RelativeTimeFormat('zh', { numeric: 'always', style: 'narrow'});
    value = relativetimefmt.format(1, 'quarter');
    arktest.assertEQ(value, '1个季度后');
    arktest.assertEQ(relativetimefmt.resolvedOptions().numeric, 'always');
    arktest.assertEQ(relativetimefmt.resolvedOptions().style, 'narrow');

    relativetimefmt = new Intl.RelativeTimeFormat('en', { numeric: 'always', style: 'long'});
    value = relativetimefmt.format(1, 'quarter');
    arktest.assertEQ(value, 'in 1 quarter');
    arktest.assertEQ(relativetimefmt.resolvedOptions().numeric, 'always');
    arktest.assertEQ(relativetimefmt.resolvedOptions().style, 'long');

    relativetimefmt = new Intl.RelativeTimeFormat('zh', { numeric: 'always', style: 'long'});
    value = relativetimefmt.format(1, 'quarter');
    arktest.assertEQ(value, '1个季度后');
    arktest.assertEQ(relativetimefmt.resolvedOptions().numeric, 'always');
    arktest.assertEQ(relativetimefmt.resolvedOptions().style, 'long');
}

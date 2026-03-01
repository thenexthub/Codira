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

const success: int = 0
const fail: int = 1

function main(): int {
    let failures: int = 0
    failures += check(testMaximize(), "Test maximize")
    failures += check(testMinimize(), "Test minimize")
    failures += check(test2String(), "Test converting to string")
    failures += check(testTagAndOptions(), "Test tag u-extensions & options")
    failures += check(testIncorrect(), "Test incorrect data")
    failures += check(testIncorrectWithoutErr(), "Test incorrect data but without error")
    failures += check(testCreateWithOtherLocale(), "Test create with other locale")
    failures += check(testManyExtensions(), "Test create with many extensions in tag")
    failures += check(testUpperCaseExtensions(), "Test ignore case-sensitivity in locale tag")
    return failures
}

function check(result: int, message: string): int {
    if (result == 0) {
        return success
    }
    return fail
}

function processCreateLocaleError(createLocaleFunction : () => Intl.Locale, msg ?: string) {
    let assertOccurred = false
    try {
        createLocaleFunction()
    }
    catch (e) {
        arktest.assertTrue(e instanceof RangeError)
        arktest.assertEQ((e as RangeError).message, (msg != undefined ? msg : errMsg))
        assertOccurred = true
    }
    finally {
        arktest.assertTrue(assertOccurred)
    }
}

function processInvalidValues(createLocaleFunction : ((arg : string) => Intl.Locale), msg ?: string) {
    const incorrectValues: ReadonlyArray<string> = Array.of<string>("", " ", "78", "Err", "!!!", "<<>>", "invalid")
    incorrectValues.forEach((invalidValue : string) => {
        processCreateLocaleError(() => createLocaleFunction(invalidValue), msg)
    })
}

function test2String() : int {
    {
        const l = new Intl.Locale("en-u-ca-gregory", {calendar : "buddhist"} as Intl.LocaleOptions)
        arktest.assertFalse(l.language == null)
        arktest.assertFalse(l.language == undefined)
        arktest.assertEQ(l.toString(), l.language!.toLowerCase() + "-u-ca-buddhist")
    }
    {
        const l = new Intl.Locale("en-u-kn", {numeric : false} as Intl.LocaleOptions)
        arktest.assertFalse(l.language == null)
        arktest.assertFalse(l.language == undefined)
        arktest.assertEQ(l.toString(), l.language!.toLowerCase())
    }
    {
        const l = new Intl.Locale("en-u-kn")
        arktest.assertFalse(l.language == null)
        arktest.assertFalse(l.language == undefined)
        arktest.assertEQ(l.toString(), l.language!.toLowerCase() + "-u-kn")
    }
    {
        processCreateLocaleError(()=>new Intl.Locale("ru-u-ca-gregory-u-kn-u-kf-upper"))
    }
    return success
}

function testMinimize() : int {
    let l = new Intl.Locale("en-u-ca-gregory", {calendar : "chinese"} as Intl.LocaleOptions)
    l = l.minimize()
    arktest.assertEQ(l.region, undefined)
    arktest.assertEQ(l.script, undefined)
    if (l.language != null && l.language != undefined && l.language!.toLowerCase() == "ru") {
        arktest.assertEQ(l.baseName, "ru")
        arktest.assertEQ(l.toString(), "ru-u-ca-chinese")
    }
    else {
        arktest.assertEQ(l.baseName, "en")
        arktest.assertEQ(l.toString(), "en-u-ca-chinese")
    }
    return success
}

function testMaximize() : int {
    let l = new Intl.Locale("ru", {collation : "emoji"} as Intl.LocaleOptions)
    l = l.maximize()
    if (l.language != null && l.language != undefined && l.language!.toLowerCase() == "ru") {
        arktest.assertEQ(l.region, "RU")
        arktest.assertEQ(l.script, "Cyrl")
        arktest.assertEQ(l.baseName, "ru-Cyrl-RU")
        arktest.assertEQ(l.toString(), "ru-Cyrl-RU-u-co-emoji")
    }
    else {
        if (l.language != null && l.language != undefined) {
            arktest.assertEQ(l.language!, "en")
        }
        arktest.assertEQ(l.region, "US")
        arktest.assertEQ(l.script, "Latn")
        arktest.assertEQ(l.baseName, "en-Latn-US")
        arktest.assertEQ(l.toString(), "en-Latn-US-u-co-emoji")
    }
    return success
}

// NOTE(ekaterinazaytseva) options have higher priority when u-extensions
function testTagAndOptions() : int {
    {
        // NOTE(ekaterinazaytseva) set collation in u-ext & in options
        const l = new Intl.Locale("ru-u-co-emoji", {collation : "phonebk"} as Intl.LocaleOptions)
        arktest.assertEQ(l.collation, "phonebk")
    }
    {
        // NOTE(ekaterinazaytseva) set collation in u-ext & in options
        const l = new Intl.Locale("ru-u-co-invalid", {collation : "phonebk"} as Intl.LocaleOptions)
        arktest.assertEQ(l.collation, "phonebk")
    }
    {
        // NOTE(ekaterinazaytseva) set numeric in u-ext & in options
        const l = new Intl.Locale("ru-u-kn", {numeric : false} as Intl.LocaleOptions)
        arktest.assertFalse(l.numeric as boolean)
    }
    {
        // NOTE(ekaterinazaytseva) set calendar in u-ext & in options
        const l = new Intl.Locale("ru-u-ca-invalid", {calendar : "gregory"} as Intl.LocaleOptions)
        arktest.assertEQ(l.calendar, "gregory")
    }
    {
        // NOTE(ekaterinazaytseva) set calendar in u-ext & in options
        const l = new Intl.Locale("ru-u-ca-japanese", {calendar : "gregory"} as Intl.LocaleOptions)
        arktest.assertEQ(l.calendar, "gregory")
    }
    {
        // set caseFirst in u-ext & in options
        const l = new Intl.Locale("ru-u-kf-upper", {caseFirst : "lower"} as Intl.LocaleOptions)
        arktest.assertEQ(l.caseFirst, "lower")
    }
    {
        // NOTE(ekaterinazaytseva) set caseFirst in u-ext & in options
        const l = new Intl.Locale("ru-u-kf-invalid", {caseFirst : "lower"} as Intl.LocaleOptions)
        arktest.assertEQ(l.caseFirst, "lower")
    }
    {
        // NOTE(ekaterinazaytseva) set numberingSystem in u-ext & in options
        const l = new Intl.Locale("ru-u-nu-invalid", {numberingSystem : "arab"} as Intl.LocaleOptions)
        arktest.assertEQ(l.numberingSystem, "arab")
    }
    {
        // set numberingSystem in u-ext & in options
        processCreateLocaleError(() => new Intl.Locale("ru-u-nu-arab", {numberingSystem : "invalid"} as Intl.LocaleOptions))
    }
    {
        // NOTE(ekaterinazaytseva) set numberingSystem in u-ext & in options
        const l = new Intl.Locale("ru-u-nu-arab", {numberingSystem : "adlm"} as Intl.LocaleOptions)
        arktest.assertEQ(l.numberingSystem, "adlm")
    }
    {
        // NOTE(ekaterinazaytseva) lang conflict
        const l = new Intl.Locale("ru-Cyrl", {language: "de"} as Intl.LocaleOptions)
        arktest.assertEQ(l.language, "de")
        arktest.assertEQ(l.script, "Cyrl")
    }
    return success
}

const errMsg = "Incorrect locale information provided"
function testIncorrect() : int {
    {
        // NOTE(ekaterinazaytseva) invalid lang
        processCreateLocaleError(() =>  new Intl.Locale("err-ru-Cyrl-u-kn-false"))
        processCreateLocaleError(() => new Intl.Locale("ru-err-u-kn-false"))
    }
    {
        // NOTE(ekaterinazaytseva) invalid region
        processInvalidValues((invalidValue : string) => { return new Intl.Locale("ru", {region : invalidValue} as Intl.LocaleOptions)} )
    }
    {
        // NOTE(ekaterinazaytseva) invalid script
        processCreateLocaleError(() => new Intl.Locale("ru-ru-err-u-kn-false"))
        processCreateLocaleError(() => new Intl.Locale("ru", {script : "Err"} as Intl.LocaleOptions))
    }
    {
        // NOTE(ekaterinazaytseva) incorrect symbols in tag
        const incorrectSymbols: ReadonlyArray<string> = Array.of<string>(" ", "*", "!", "?", "_", "~", "@", "\\", "<", ">", ".", "\,")
        incorrectSymbols.forEach((sym : string) => {
        const tag = "Ru" + sym
        processCreateLocaleError(() => new Intl.Locale(tag))
      })
    }
    {
        processCreateLocaleError(() => new Intl.Locale(""), "First argument to Intl.Locale constructor can't be empty or missing")
    }
    {
        const incorrectValues: ReadonlyArray<string> = Array.of<string>("r", "1999", "19", "errrrrrrr")
        incorrectValues.forEach((incorrectValue : string) => {
            processCreateLocaleError(()=> new Intl.Locale(incorrectValue))
        })
    }
    return success
}

function testIncorrectWithoutErr() : int {
    // NOTE(ekaterinazaytseva) false correct - lang match [A-Z,a-z]{2,8}
    {
        // NOTE(ekaterinazaytseva) invalid lang - but no err on it
        const falseCorrectValues: ReadonlyArray<string> = Array.of<string>("er", "err", "errrrrrr")
        falseCorrectValues.forEach((correctValue : string) => {
            const l = new Intl.Locale(correctValue)
            arktest.assertEQ(l.language, correctValue)
            arktest.assertEQ(l.toString(), correctValue)
        })
    }
    {
        const l = new Intl.Locale("ru", {language : "Err"} as Intl.LocaleOptions)
        arktest.assertEQ(l.language, "Err")
        arktest.assertEQ(l.toString(), "Err")
    }
    return success
}

function testCreateWithOtherLocale() : int {
    {
        const l = new Intl.Locale("ru", {numeric : true, collation : "emoji"} as Intl.LocaleOptions)
        arktest.assertTrue(l.numeric as boolean)
        const l1 = new Intl.Locale(l, {numeric : false,} as Intl.LocaleOptions)
        arktest.assertFalse(l1.numeric as boolean)
        arktest.assertEQ(l.language, (l1.language))
        arktest.assertEQ(l.collation, (l1.collation))
        arktest.assertEQ(l.numberingSystem, (l1.numberingSystem))
    }
    {
        const l = new Intl.Locale("ru", {numeric : true, collation : "emoji"} as Intl.LocaleOptions)
        arktest.assertTrue(l.numeric as boolean)
        const l1 = new Intl.Locale(l, {numeric : false,} as Intl.LocaleOptions)
        arktest.assertFalse(l1.numeric as boolean)
        arktest.assertEQ(l.language, (l1.language))
        arktest.assertEQ(l.collation, (l1.collation))
        arktest.assertEQ(l.numberingSystem, (l1.numberingSystem))
    }
    return success
}

function testManyExtensions() : int {
    // NOTE(ekaterinazaytseva) only 1 u-extension in tag is allowed
    processCreateLocaleError(() => new Intl.Locale("ru-u-ca-gregory-u-nu-arab"))
    {
        const validExtensions: ReadonlyArray<string> = Array.of<string>("-u-ca-gregory", "-u-nu-arab", "-u-kn", "-u-co-emoji", "-u-kf-upper", "-u-hc-h12")
        for (let i = 0;  i < validExtensions.length; ++i) {
			for (let j = 0; j < validExtensions.length; ++j) {
                if (i == j)
                    continue
                let tag = "ru" + validExtensions[i] + validExtensions[j]
                processCreateLocaleError(() => { return new Intl.Locale(tag)} )
            }
        }
    }
    return success
}

function testUpperCaseExtensions() : int {
    {
        let l = new Intl.Locale("ru-U-KN-TRUE")
        arktest.assertEQ(l.numeric, true)
    }
    {
        let l = new Intl.Locale("ru-u-KN-TRUE")
        arktest.assertEQ(l.numeric, true)
    }
    {
        let l = new Intl.Locale("ru-u-KN-true")
        arktest.assertEQ(l.numeric, true)
    }
    return success
}

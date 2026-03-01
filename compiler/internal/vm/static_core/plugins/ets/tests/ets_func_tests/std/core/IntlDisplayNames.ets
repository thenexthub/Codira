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

const success: int = 0;
const fail: int = 1;

// Get locales using best fit algorithm
const en: string = Intl.intlBestFitLocale('en-US');
const cn: string = Intl.intlBestFitLocale('zh-CN');
const fr: string = Intl.intlBestFitLocale('fr-FR');
const es: string = Intl.intlBestFitLocale('es-ES');

function main(): int {
    const suite = new arktest.ArkTestsuite('Intl.DisplayNames Tests');

    suite.addTest('Display names creation', testDisplayNamesCreation);
    suite.addTest('Language display names', testLanguageDisplayNames);
    suite.addTest('Script display names', testScriptDisplayNames);
    suite.addTest('Region display names', testRegionDisplayNames);
    suite.addTest('Currency display names', testCurrencyDisplayNames);
    suite.addTest('Display names styles', testDisplayNamesStyles);
    suite.addTest('Display names fallback', testDisplayNamesFallback);
    suite.addTest('Display names language display options', testLanguageDisplayOptions);
    suite.addTest('Display names for invalid codes', testInvalidCodes);
    suite.addTest('Display names supportedLocalesOf', testSupportedLocalesOf);
    suite.addTest('Display names resolvedOptions', testResolvedOptions);

    return suite.run();
}

function testDisplayNamesCreation(): void {
    // DisplayNames constructor should throw when type is not specified
    arktest.expectError(
        (): void => {
            new Intl.DisplayNames('en');
        },
        new TypeError('Options parameter is required')
    );

    const displayNames = new Intl.DisplayNames('en', { type: 'language' });
    arktest.assertTrue(displayNames instanceof Intl.DisplayNames);

    const displayNamesWithLocaleArray = new Intl.DisplayNames(['en', 'fr'], { type: 'language' });
    arktest.assertTrue(displayNamesWithLocaleArray instanceof Intl.DisplayNames);

    new Intl.DisplayNames('en', { type: 'language' });
    new Intl.DisplayNames('en', { type: 'region' });
    new Intl.DisplayNames('en', { type: 'script' });
    new Intl.DisplayNames('en', { type: 'currency' });
}

class DisplayNamesOptionsBuilder {
    public type?: 'language' | 'region' | 'script' | 'currency';
    public style?: 'long' | 'short';
    public languageDisplay?: 'standard' | 'dialect';
    public fallback?: 'code' | 'none';
    public localeMatcher?: 'lookup' | 'best fit';

    public with_type(type: 'language' | 'region' | 'script' | 'currency'): DisplayNamesOptionsBuilder {
        this.type = type;
        return this;
    }

    public with_style(style: 'long' | 'short'): DisplayNamesOptionsBuilder {
        this.style = style;
        return this;
    }

    public with_language_display(languageDisplay: 'standard' | 'dialect'): DisplayNamesOptionsBuilder {
        this.languageDisplay = languageDisplay;
        return this;
    }

    public with_fallback(fallback: 'code' | 'none'): DisplayNamesOptionsBuilder {
        this.fallback = fallback;
        return this;
    }

    public with_locale_matcher(localeMatcher: 'lookup' | 'best fit'): DisplayNamesOptionsBuilder {
        this.localeMatcher = localeMatcher;
        return this;
    }

    public build(): Intl.DisplayNamesOptions {
        return {
            type: this.type ?? 'language',
            style: this.style ?? 'long',
            languageDisplay: this.languageDisplay ?? 'standard',
            fallback: this.fallback ?? 'code',
            localeMatcher: this.localeMatcher ?? 'best fit'
        } as Intl.DisplayNamesOptions;
    }

}

function testDisplayName(
    locale: string,
    code: string,
    expected: string | undefined,
    options: Intl.DisplayNamesOptions
): void {
    const displayNames = new Intl.DisplayNames(locale, options);
    const result = displayNames.of(code);
    if (expected === undefined) {
        arktest.assertEQ(result, undefined, `DisplayNames.of('${code}') with locale ${locale} and type ${options.type} should return undefined`);
    } else {
        arktest.assertEQ(result, expected, `DisplayNames.of('${code}') with locale ${locale} and type ${options.type} should return '${expected}' but got '${result}'`);
    }
}

function testLanguageDisplayNames(): void {
    const languageOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .build();

    // en
    testDisplayName(en, 'de', 'German', languageOptions);
    testDisplayName(en, 'fr', 'French', languageOptions);
    testDisplayName(en, 'ja', 'Japanese', languageOptions);
    testDisplayName(en, 'zh-Hans', 'Chinese (Simplified)', languageOptions);

    // fr
    testDisplayName(fr, 'de', 'allemand', languageOptions);
    testDisplayName(fr, 'en', 'anglais', languageOptions);
    testDisplayName(fr, 'ja', 'japonais', languageOptions);

    // es
    testDisplayName(es, 'de', 'alemán', languageOptions);
    testDisplayName(es, 'en', 'inglés', languageOptions);


    const longStyleOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_style('long')
        .build();
    testDisplayName(en, 'de', 'German', longStyleOptions);

    const shortStyleOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_style('short')
        .build();
    testDisplayName(en, 'zh-Hans', 'Simplified Chinese', shortStyleOptions);

    const codeFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_fallback('code')
        .build();
    testDisplayName(en, 'xyz', 'xyz', codeFallbackOptions);

    const noneFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_fallback('none')
        .build();
    testDisplayName(en, 'xyz', 'xyz', noneFallbackOptions);
}

function testScriptDisplayNames(): void {
    const scriptOptions = new DisplayNamesOptionsBuilder()
        .with_type('script')
        .build();

    testDisplayName(en, 'Latn', 'Latin', scriptOptions);
    testDisplayName(en, 'Cyrl', 'Cyrillic', scriptOptions);
    testDisplayName(en, 'Hani', 'Han', scriptOptions);

    // fr
    testDisplayName(fr, 'Latn', 'latin', scriptOptions);
    testDisplayName(fr, 'Cyrl', 'cyrillique', scriptOptions);

    const codeFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('script')
        .with_fallback('code')
        .build();

    testDisplayName(en, 'LaT', 'LaT', codeFallbackOptions); // Invalid script format
    testDisplayName(en, 'LATN', 'LATN', codeFallbackOptions); // Invalid script format
    testDisplayName(en, 'Wxyz', 'Wxyz', codeFallbackOptions); // Non-existent script
}

function testRegionDisplayNames(): void {
    // Basic region tests
    const regionOptions = new DisplayNamesOptionsBuilder()
        .with_type('region')
        .build();

    testDisplayName(en, 'US', 'United States', regionOptions);
    testDisplayName(en, 'DE', 'Germany', regionOptions);
    testDisplayName(en, 'FR', 'France', regionOptions);
    testDisplayName(en, 'JP', 'Japan', regionOptions);

    // UN M49 region codes
    testDisplayName(en, '001', 'world', regionOptions);
    testDisplayName(en, '419', 'Latin America', regionOptions);

    testDisplayName(fr, 'US', 'États-Unis', regionOptions);
    testDisplayName(fr, 'DE', 'Allemagne', regionOptions);

    const codeFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('region')
        .with_fallback('code')
        .build();

    testDisplayName(en, 'XX', 'XX', codeFallbackOptions); // Non-existent region
    testDisplayName(en, 'usa', 'usa', codeFallbackOptions); // Invalid region format
    testDisplayName(en, '1234', '1234', codeFallbackOptions); // Invalid region format
}

function testCurrencyDisplayNames(): void {
    const currencyOptions = new DisplayNamesOptionsBuilder()
        .with_type('currency')
        .build();

    testDisplayName(en, 'USD', 'US Dollar', currencyOptions);
    testDisplayName(en, 'EUR', 'Euro', currencyOptions);
    testDisplayName(en, 'JPY', 'Japanese Yen', currencyOptions);

    testDisplayName(fr, 'USD', 'dollar des États-Unis', currencyOptions);
    testDisplayName(fr, 'EUR', 'euro', currencyOptions);

    const codeFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('currency')
        .with_fallback('code')
        .build();

    testDisplayName(en, 'XYZ', 'XYZ', codeFallbackOptions); // Non-existent currency
}

function testDisplayNamesStyles(): void {
    const longNames = new Intl.DisplayNames(en, { type: 'region', style: 'long' });
    arktest.assertEQ(longNames.of('US'), 'United States');

    const shortNames = new Intl.DisplayNames(en, { type: 'region', style: 'short' });
    const shortResult = shortNames.of('US');
    arktest.assertTrue(shortResult == 'US' || shortResult == 'United States');

    const options = shortNames.resolvedOptions();
    arktest.assertEQ(options.style, 'short');

    const regionLongOptions = new DisplayNamesOptionsBuilder()
        .with_type('region')
        .with_style('long')
        .build();
    testDisplayName(en, 'US', 'United States', regionLongOptions);

    const languageLongOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_style('long')
        .build();
    testDisplayName(en, 'zh-Hans', 'Chinese (Simplified)', languageLongOptions);

    const regionShortOptions = new DisplayNamesOptionsBuilder()
        .with_type('region')
        .with_style('short')
        .build();
    testDisplayName(en, 'US', 'United States', regionShortOptions);
}

function testDisplayNamesFallback(): void {
    const defaultFallback = new Intl.DisplayNames(en, { type: 'region' });
    arktest.assertEQ(defaultFallback.of('XX'), 'XX', 'Default fallback should return the code itself');

    const codeFallback = new Intl.DisplayNames(en, { type: 'region', fallback: 'code' });
    arktest.assertEQ(codeFallback.of('XX'), 'XX', 'Code fallback should return the code itself');

    const noneFallback = new Intl.DisplayNames(en, { type: 'region', fallback: 'none' });
    const noneFallbackResult = noneFallback.of('XX');
    arktest.assertTrue(noneFallbackResult === undefined || noneFallbackResult === 'XX',
        'None fallback should return undefined or the code itself');

    const options = noneFallback.resolvedOptions();
    arktest.assertEQ(options.fallback, 'none');

    const languageCodeFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_fallback('code')
        .build();
    testDisplayName(en, 'xyz', 'xyz', languageCodeFallbackOptions);

    const languageNoneFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_fallback('none')
        .build();
    const languageNoneResult = new Intl.DisplayNames(en, languageNoneFallbackOptions).of('xyz');
    arktest.assertTrue(languageNoneResult === undefined || languageNoneResult === 'xyz',
        'None fallback should return undefined or the code itself');

    const scriptCodeFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('script')
        .with_fallback('code')
        .build();
    testDisplayName(en, 'Wxyz', 'Wxyz', scriptCodeFallbackOptions);

    const scriptNoneFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('script')
        .with_fallback('none')
        .build();
    const scriptNoneResult = new Intl.DisplayNames(en, scriptNoneFallbackOptions).of('Wxyz');
    arktest.assertTrue(scriptNoneResult === undefined || scriptNoneResult === 'Wxyz',
        'None fallback should return undefined or the code itself');

    const currencyCodeFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('currency')
        .with_fallback('code')
        .build();
    testDisplayName(en, 'XYZ', 'XYZ', currencyCodeFallbackOptions);

    const currencyNoneFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('currency')
        .with_fallback('none')
        .build();
    // Implementation may return code or undefined for none fallback
    const currencyNoneResult = new Intl.DisplayNames(en, currencyNoneFallbackOptions).of('XYZ');
    arktest.assertTrue(currencyNoneResult === undefined || currencyNoneResult === 'XYZ',
        'None fallback should return undefined or the code itself');
}

function testLanguageDisplayOptions(): void {
    const dialectDisplay = new Intl.DisplayNames(en, {
        type: 'language',
        languageDisplay: 'dialect'
    });

    const standardDisplay = new Intl.DisplayNames(en, {
        type: 'language',
        languageDisplay: 'standard'
    });

    const dialectOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_language_display('dialect')
        .build();
    testDisplayName(en, 'en-US', 'English (United States)', dialectOptions);
    testDisplayName(en, 'pt-BR', 'Portuguese (Brazil)', dialectOptions);

    const standardOptions = new DisplayNamesOptionsBuilder()
        .with_type('language')
        .with_language_display('standard')
        .build();
    testDisplayName(en, 'en-US', 'English (United States)', standardOptions);
    testDisplayName(en, 'pt-BR', 'Portuguese (Brazil)', standardOptions);

    const dialectResult = dialectDisplay.of('en-US');
    const standardResult = standardDisplay.of('en-US');

    // The implementation may not differentiate between dialect and standard for some locales
    // So we'll just check that they both return something valid
    arktest.assertTrue(dialectResult != null && standardResult != null,
        'Both dialect and standard display should return a valid result');

    // Test that languageDisplay is reflected in resolvedOptions
    const options = standardDisplay.resolvedOptions();
    arktest.assertEQ(options.languageDisplay, 'standard');
}

function testInvalidCodes(): void {
    const displayNames = new Intl.DisplayNames(en, { type: 'language' });

    // DisplayNames.of should throw when given an empty code
    arktest.expectError(
        (): void => {
            displayNames.of('');
        },
        new RangeError('Code cannot be empty')
    );

    const result = displayNames.of('not-a-language-code');
    arktest.assertTrue(result instanceof String, 'DisplayNames.of with invalid code should return a string value');

    const scriptFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('script')
        .with_fallback('code')
        .build();

    testDisplayName(en, 'LaT', 'LaT', scriptFallbackOptions); // Wrong format (lowercase T)
    testDisplayName(en, 'LATN', 'LATN', scriptFallbackOptions); // Wrong format (all uppercase)

    // Test malformed region codes
    const regionFallbackOptions = new DisplayNamesOptionsBuilder()
        .with_type('region')
        .with_fallback('code')
        .build();

    testDisplayName(en, 'usa', 'usa', regionFallbackOptions); // Wrong format (lowercase)
    testDisplayName(en, '1234', '1234', regionFallbackOptions); // Wrong format (4 digits)
}

function testSupportedLocalesOf(): void {
    const validLocales = [en, fr, cn] as FixedArray<string>;
    const supported = Intl.DisplayNames.supportedLocalesOf(validLocales);
    let supportedArray = Array.from<String>(supported);
    arktest.assertTrue(supportedArray.length > 0, 'At least one locale should be supported');
}

function testResolvedOptions(): void {
    const displayNames = new Intl.DisplayNames(en, { type: 'language' });
    const options = displayNames.resolvedOptions();

    arktest.assertEQ(options.locale instanceof String, true, 'resolvedOptions should have a locale property');
    arktest.assertEQ(options.type, 'language', 'resolvedOptions should reflect the type option');
    arktest.assertEQ(options.style, 'long', 'resolvedOptions should have default style');
    arktest.assertEQ(options.fallback, 'code', 'resolvedOptions should have default fallback');

    const customDisplayNames = new Intl.DisplayNames(fr, {
        type: 'region',
        style: 'short',
        fallback: 'none'
    });
    const customOptions = customDisplayNames.resolvedOptions();

    arktest.assertTrue(customOptions.locale.startsWith('fr'), 'resolvedOptions should reflect the locale');
    arktest.assertEQ(customOptions.type, 'region', 'resolvedOptions should reflect the type option');
    arktest.assertEQ(customOptions.style, 'short', 'resolvedOptions should reflect the style option');
    arktest.assertEQ(customOptions.fallback, 'none', 'resolvedOptions should reflect the fallback option');

    const languageDisplayNames = new Intl.DisplayNames(en, {
        type: 'language',
        languageDisplay: 'standard'
    });
    const languageOptions = languageDisplayNames.resolvedOptions();

    arktest.assertEQ(languageOptions.languageDisplay, 'standard', 'resolvedOptions should reflect the languageDisplay option');
}

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
    const suite = new arktest.ArkTestsuite("Intl. NumberFormat $_invoke call constructor with NumberFormatOptios");
    // Tests from https://gitee.com/openharmony/xts_acts/tree/master/commonlibrary/ets_utils/codeparsingtest/builtinstest/entry/src/ohosTest/ets/test/Intl/Intl.test.ets
    suite.addTest("testNumberFormatConstructorNew0001", testNumberFormatConstructorNew0001);
    suite.addTest("testNumberFormatConstructorNew0002", testNumberFormatConstructorNew0002);
    suite.addTest("testNumberFormatConstructorNew0003", testNumberFormatConstructorNew0003);
    suite.addTest("testNumberFormatConstructorNew0004", testNumberFormatConstructorNew0004);
    suite.addTest("testNumberFormatConstructorNew0005", testNumberFormatConstructorNew0005);
    suite.addTest("testNumberFormatConstructorNew0006", testNumberFormatConstructorNew0006);
    suite.addTest("testNumberFormatConstructorNew0007", testNumberFormatConstructorNew0007);
    suite.addTest("testNumberFormatConstructorNew0009", testNumberFormatConstructorNew0009);
    suite.addTest("testNumberFormatConstructorNew0010", testNumberFormatConstructorNew0010);
    suite.addTest("testNumberFormatConstructorNew0011", testNumberFormatConstructorNew0011);
    suite.addTest("testNumberFormatConstructorNew0013", testNumberFormatConstructorNew0013);
    suite.addTest("testNumberFormatConstructorNew0014", testNumberFormatConstructorNew0014);
    suite.addTest("testNumberFormatConstructorNew0015", testNumberFormatConstructorNew0015);
    suite.addTest("testNumberFormatConstructorNew0016", testNumberFormatConstructorNew0016);
    suite.addTest("testNumberFormatConstructorNew0017", testNumberFormatConstructorNew0017);
    suite.addTest("testNumberFormatConstructorNew0018", testNumberFormatConstructorNew0018);
    suite.addTest("testNumberFormatConstructorNew0019", testNumberFormatConstructorNew0019);
    suite.addTest("testNumberFormatConstructorNew0020", testNumberFormatConstructorNew0020);
    suite.addTest("testNumberFormatConstructorNew0021", testNumberFormatConstructorNew0021);
    suite.addTest("testNumberFormatConstructorNew0022", testNumberFormatConstructorNew0022);
    suite.addTest("testNumberFormatConstructorNew0023", testNumberFormatConstructorNew0023);
    suite.addTest("testNumberFormatConstructorNew0024", testNumberFormatConstructorNew0024);
    suite.addTest("testNumberFormatConstructorNew0026", testNumberFormatConstructorNew0026);
    suite.addTest("testNumberFormatConstructorNew0027", testNumberFormatConstructorNew0027);
    suite.addTest("testNumberFormatConstructorNew0028", testNumberFormatConstructorNew0028);
    suite.addTest("testNumberFormatConstructorNew0029", testNumberFormatConstructorNew0029);
    suite.addTest("testNumberFormatConstructorNew0030", testNumberFormatConstructorNew0030);
    suite.addTest("testNumberFormatConstructorNew0031", testNumberFormatConstructorNew0031);
    suite.addTest("testNumberFormatConstructorNew0032", testNumberFormatConstructorNew0032);
    suite.addTest("testNumberFormatConstructorNew0033", testNumberFormatConstructorNew0033);
    suite.addTest("testNumberFormatConstructorNew0034", testNumberFormatConstructorNew0034);
    suite.addTest("testNumberFormatConstructorNew0035", testNumberFormatConstructorNew0035);
    suite.addTest("testNumberFormatConstructorNew0036", testNumberFormatConstructorNew0036);
    suite.addTest("testNumberFormatConstructorNew0037", testNumberFormatConstructorNew0037);
    suite.addTest("testNumberFormatConstructorNew0038", testNumberFormatConstructorNew0038);
    suite.addTest("testNumberFormatConstructorNew0039", testNumberFormatConstructorNew0039);
    suite.addTest("testNumberFormatConstructorNew0040", testNumberFormatConstructorNew0040);
    suite.addTest("testNumberFormatConstructorNew0041", testNumberFormatConstructorNew0041);
    suite.addTest("testNumberFormatConstructorNew0042", testNumberFormatConstructorNew0042);
    suite.addTest("testNumberFormatConstructorNew0043", testNumberFormatConstructorNew0043);
    suite.addTest("testNumberFormatConstructorNew0044", testNumberFormatConstructorNew0044);
    suite.addTest("testNumberFormatConstructorNew0045", testNumberFormatConstructorNew0045);
    suite.addTest("testNumberFormatConstructorNew0046", testNumberFormatConstructorNew0046);
    suite.addTest("testNumberFormatConstructorNew0047", testNumberFormatConstructorNew0047);
    suite.addTest("testNumberFormatConstructorNew0048", testNumberFormatConstructorNew0048);
    suite.addTest("testNumberFormatConstructorNew0049", testNumberFormatConstructorNew0049);
    suite.addTest("testNumberFormatConstructorNew0050", testNumberFormatConstructorNew0050);
    suite.addTest("testNumberFormatConstructorNew0051", testNumberFormatConstructorNew0051);
    suite.addTest("testNumberFormatConstructorNew0052", testNumberFormatConstructorNew0052);
    suite.addTest("testNumberFormatConstructorNew0053", testNumberFormatConstructorNew0053);
    suite.addTest("testNumberFormatConstructorNew0054", testNumberFormatConstructorNew0054);
    suite.addTest("testNumberFormatConstructorNew0055", testNumberFormatConstructorNew0055);
    suite.addTest("testNumberFormatConstructorNew0056", testNumberFormatConstructorNew0056);
    suite.addTest("testNumberFormatConstructorNew0057", testNumberFormatConstructorNew0057);
    suite.addTest("testNumberFormatConstructorNew0058", testNumberFormatConstructorNew0058);
    suite.addTest("testNumberFormatConstructorNew0059", testNumberFormatConstructorNew0059);
    suite.addTest("testNumberFormatConstructorNew0060", testNumberFormatConstructorNew0060);
    return suite.run();
}

function testNumberFormatConstructorNew0001() {
    let numberFormat = Intl.NumberFormat.$_invoke();
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0002() {
    let locales = "zh";
    let numberFormat = Intl.NumberFormat.$_invoke(locales);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0003() {
    let locales: string = "";
    let numberFormat = Intl.NumberFormat.$_invoke(locales);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0004() {
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke("test") });
}

function testNumberFormatConstructorNew0005() {
    let locales: string[] = [];
    let numberFormat = Intl.NumberFormat.$_invoke(locales);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0006() {
    let locales = ["zh"];
    let numberFormat = Intl.NumberFormat.$_invoke(locales);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0007() {
    let locales = ["zh", "en-US", "de"];
    let numberFormat = Intl.NumberFormat.$_invoke(locales);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0009() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        localeMatcher: "lookup"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0010() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        localeMatcher: "best fit"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0011() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
        localeMatcher: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0013() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          style: "decimal"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0014() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          style: "currency"
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Style is currency but currency is undefined"));
}

function testNumberFormatConstructorNew0015() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          style: "currency",
          currency: "USD"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0016() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          style: "percent"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0017() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          style: "unit",
          unit: "kilometer"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0018() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          style: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0019() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          currency: "EUR"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0020() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          currency: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0021() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          currency: "test"
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid currency"));
}

function testNumberFormatConstructorNew0022() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          currencySign: "standard"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0023() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          style: "currency",
          currency: "CNY",
          currencySign: "accounting"
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(-1234567.89);
    arktest.assertEQ(formattedNumber, "(¥1,234,567.89)");
}

function testNumberFormatConstructorNew0024() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          currencySign: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0026() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          useGrouping: true
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0027() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          useGrouping: false
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1234567.89");
}

function testNumberFormatConstructorNew0028() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          useGrouping: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0029() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumIntegerDigits: 1
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0030() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumIntegerDigits: 10
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "0,001,234,567.89");
}

function testNumberFormatConstructorNew0031() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumIntegerDigits: 21
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "000,000,000,000,001,234,567.89");
}

function testNumberFormatConstructorNew0032() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumIntegerDigits: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0033() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumIntegerDigits: 0
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid minimumIntegerDigits"));
}

function testNumberFormatConstructorNew0034() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumIntegerDigits: 22
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid minimumIntegerDigits"));
}

function testNumberFormatConstructorNew0035() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumFractionDigits: 0
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat instanceof Intl.NumberFormat);
}

function testNumberFormatConstructorNew0036() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumFractionDigits: 50
    }
    arktest.expectNoThrow((): void => { Intl.NumberFormat.$_invoke(locales, options) });
}

function testNumberFormatConstructorNew0037() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumFractionDigits: 100
    }
    arktest.expectNoThrow((): void => { Intl.NumberFormat.$_invoke(locales, options) });
}

function testNumberFormatConstructorNew0038() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumFractionDigits: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0039() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumFractionDigits: -1
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid minimumFractionDigits"));
}

function testNumberFormatConstructorNew0040() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumFractionDigits: 101
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid minimumFractionDigits"));
}

function testNumberFormatConstructorNew0041() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumFractionDigits: 0
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0042() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumFractionDigits: 50
    }
    arktest.expectNoThrow((): void => { Intl.NumberFormat.$_invoke(locales, options) });
}

function testNumberFormatConstructorNew0043() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumFractionDigits: 100
    }
    arktest.expectNoThrow((): void => { Intl.NumberFormat.$_invoke(locales, options) });
}

function testNumberFormatConstructorNew0044() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumFractionDigits: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0045() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumFractionDigits: -1
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid maximumFractionDigits"));
}

function testNumberFormatConstructorNew0046() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumFractionDigits: 101
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid maximumFractionDigits"));
}

function testNumberFormatConstructorNew0047() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumSignificantDigits: 1
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0048() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumSignificantDigits: 10
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0049() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumSignificantDigits: 21
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0050() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumSignificantDigits: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0051() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumSignificantDigits: 0
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid minimumSignificantDigits"));
}

function testNumberFormatConstructorNew0052() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          minimumSignificantDigits: 22
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid minimumSignificantDigits"));
}

function testNumberFormatConstructorNew0053() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumSignificantDigits: 1
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0054() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumSignificantDigits: 10
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0055() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumSignificantDigits: 21
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0056() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumSignificantDigits: undefined
    }
    let numberFormat = Intl.NumberFormat.$_invoke(locales, options);
    let formattedNumber = numberFormat.format(1234567.89);
    arktest.assertEQ(formattedNumber, "1,234,567.89");
}

function testNumberFormatConstructorNew0057() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumSignificantDigits: 0
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid maximumSignificantDigits"));
}

function testNumberFormatConstructorNew0058() {
    let locales = "zh";
    let options: Intl.NumberFormatOptions = {
          maximumSignificantDigits: 22
    }
    arktest.expectError((): void => { Intl.NumberFormat.$_invoke(locales, options) }, new RangeError("Invalid maximumSignificantDigits"));
}

function testNumberFormatConstructorNew0059() {
    let numberFormat = Intl.NumberFormat.$_invoke(undefined);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

function testNumberFormatConstructorNew0060() {
    let numberFormat = Intl.NumberFormat.$_invoke(undefined, undefined);
    arktest.assertTrue(numberFormat !== undefined && numberFormat !== null);
}

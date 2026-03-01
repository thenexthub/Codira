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

function testBigIntPow() {
    let base0: BigInt = new BigInt(0)
    let base1: BigInt = new BigInt(1)
    let base2: BigInt = new BigInt(2)
    let base7: BigInt = new BigInt(7)
    let base10: BigInt = new BigInt(10)
    let base10000000000: BigInt = new BigInt(10000000000)
    let baseNegative1: BigInt = new BigInt(-1)
    let baseNegative2: BigInt = new BigInt(-2)
    let baseNegative7: BigInt = new BigInt(-7)
    let baseNegative10: BigInt = new BigInt(-10)
    let baseNegative10000000000: BigInt = new BigInt(-10000000000)

    let exp0: BigInt = new BigInt(0)
    let exp1: BigInt = new BigInt(1)
    let exp2: BigInt = new BigInt(2)
    let exp7: BigInt = new BigInt(7)
    let exp10: BigInt = new BigInt(10)
    let exp100: BigInt = new BigInt(100)
    let exp10000000000: BigInt = new BigInt(10000000000)
    let expNegative1: BigInt = new BigInt(-1)
    let expNegative10: BigInt = new BigInt(-10)
    let expNegative10000000000: BigInt = new BigInt(-10000000000)

    let tmp: BigInt;
    let result: BigInt;

    const expectedRangeError = (e: Error): boolean => {
        return (e instanceof RangeError)
    }

    // 1^-1 throw Error
    arktest.expectThrow(() => { base1.pow(expNegative1) }, expectedRangeError)
    arktest.expectThrow(() => { base1 ** expNegative1 }, expectedRangeError)

    // 1^0 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(base1.pow(exp0)));

    // -1^0 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(baseNegative1.pow(exp0)));

    // 0^0 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(base0.pow(exp0)));

    // 10^0 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(base10.pow(exp0)));

    // -10^0 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(baseNegative10.pow(exp0)));

    // 10000000000^0 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(base10000000000.pow(exp0)));

    // -10000000000^0 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(baseNegative10000000000.pow(exp0)));

    // 1^1 = 1
    result = new BigInt(1)
    arktest.assertTrue(result.equals(base1.pow(exp1)));

    // 10^1 = 10
    result = new BigInt(10)
    arktest.assertTrue(result.equals(base10.pow(exp1)));

    // 10000000000^1 = 10000000000
    result = new BigInt(10000000000)
    arktest.assertTrue(result.equals(base10000000000.pow(exp1)));

    // 2^2 = 4
    result = new BigInt(4)
    arktest.assertTrue(result.equals(base2.pow(exp2)));

    // 2^7 = 128
    result = new BigInt(128)
    arktest.assertTrue(result.equals(base2.pow(exp7)));

    // 2^100 = 1267650600228229401496703205376
    result = new BigInt("1267650600228229401496703205376")
    arktest.assertTrue(result.equals(base2.pow(exp100)));

    // -7^7 = -823543
    result = new BigInt(-823543)
    arktest.assertTrue(result.equals(baseNegative7.pow(exp7)));

    // -2^101 = 1267650600228229401496703205376
    result = new BigInt("-2535301200456458802993406410752")
    arktest.assertTrue(result.equals(baseNegative2.pow(exp100 + base1)));

    // -7^100 = 3234476509624757991344647769100216810857203198904625400933895331391691459636928060001
    result = new BigInt("3234476509624757991344647769100216810857203198904625400933895331391691459636928060001")
    arktest.assertTrue(result.equals(baseNegative7.pow(exp100)));

    // 2^2 = 4 and test frontend "a ** b"
    result = new BigInt("4")
    tmp = base2 ** exp2
    arktest.assertTrue(result == tmp);

    // 4^2 = 16 and test frontend "a **= b"
    tmp = 4n
    result = new BigInt("16")
    tmp **= exp2
    arktest.assertTrue(result == tmp);
}

function testBigIntStringBin() {
    arktest.assertEQ(new BigInt("0b1111"), 15n);
    arktest.assertEQ(new BigInt("0b10"), 2n);
    arktest.assertEQ(new BigInt("0b0"), 0n);
    arktest.assertEQ(new BigInt("0b1"), 1n);

    let binaryString = "0b1";
    for (let i = 0; i < 128; i++) {
        binaryString += "0";
    }
    arktest.assertEQ(new BigInt(binaryString), 340282366920938463463374607431768211456n);

    arktest.assertEQ(new BigInt("0B1111"), 15n);
    arktest.assertEQ(new BigInt("0B10"), 2n);
    arktest.assertEQ(new BigInt("0B0"), 0n);
    arktest.assertEQ(new BigInt("0B1"), 1n);

    binaryString = "0B1";
    for (let i = 0; i < 128; i++) {
        binaryString += "0";
    }
    arktest.assertEQ(new BigInt(binaryString), 340282366920938463463374607431768211456n);
}

function testBigIntStringOct() {
    arktest.assertEQ(new BigInt("0o7"), 7n);
    arktest.assertEQ(new BigInt("0o10"), 8n);
    arktest.assertEQ(new BigInt("0o20"), 16n);
    arktest.assertEQ(new BigInt("0O7"), 7n);
    arktest.assertEQ(new BigInt("0O10"), 8n);
    arktest.assertEQ(new BigInt("0O20"), 16n);
}

function testBigIntStringDec() {
    arktest.assertEQ(new BigInt("10"), 10n);
    arktest.assertEQ(new BigInt("18446744073709551616"), 18446744073709551616n);
    arktest.assertEQ(new BigInt("7"), 7n);
    arktest.assertEQ(new BigInt("88"), 88n);
    arktest.assertEQ(new BigInt("900"), 900n);
    arktest.assertEQ(new BigInt("-10"), -10n);
    arktest.assertEQ(new BigInt("-18446744073709551616"), -18446744073709551616n);
    arktest.assertEQ(new BigInt("-7"), -7n);
    arktest.assertEQ(new BigInt("-88"), -88n);
    arktest.assertEQ(new BigInt("-900"), -900n);
    arktest.assertEQ(new BigInt("100.0"), 100n);
    arktest.assertEQ(new BigInt("100.000"), 100n);
}

function testBigIntStringHex() {
    arktest.assertEQ(new BigInt("0xa"), 10n);
    arktest.assertEQ(new BigInt("0xff"), 255n);
    arktest.assertEQ(new BigInt("0xfabc"), 64188n);
    arktest.assertEQ(new BigInt("0xfffffffffffffffffff"), 75557863725914323419135n);
    arktest.assertEQ(new BigInt("0Xa"), 10n);
    arktest.assertEQ(new BigInt("0Xff"), 255n);
    arktest.assertEQ(new BigInt("0Xfabc"), 64188n);
    arktest.assertEQ(new BigInt("0Xfffffffffffffffffff"), 75557863725914323419135n);
}

function testBigIntStringError() {
    const invalidBigIntStrings = [
        "10x", "10b", "0b", "-0x1", "-0XFFab", 
        "000 12", "0o", "0x", "00o", "00b", "00x", "10n"
    ];
    
    for (const str of invalidBigIntStrings) {
        arktest.expectError(() => { BigInt(str) }, "BigInt.parseFromString(): provided string is invalid")
    }

    const invalidBigIntStrings2 = [
        "10.5", "11.11", "22.22", "33.33"
    ];
    for (let str of invalidBigIntStrings2) {
        let errMess = "Can't convert number " + str + " to BigInt because it's not an integer"
        arktest.expectError(() => { BigInt(str) }, errMess)
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("BigInt.powTest");
    suite.addTest("Test BigInt pow", testBigIntPow);
    suite.addTest("Test BigInt String Bin", testBigIntStringBin);
    suite.addTest("Test BigInt String Oct", testBigIntStringOct);
    suite.addTest("Test BigInt String Dec", testBigIntStringDec);
    suite.addTest("Test BigInt String Hex", testBigIntStringHex);
    suite.addTest("Test BigInt String Error", testBigIntStringError);

    return suite.run();
}

/*
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

function testBigIntToString1() {
    let n = 10n;
    let result = n.toString(undefined);
    arktest.assertEQ(result, "10")
}

function testBigIntToString2() {
    let n = -10n;
    let result = n.toString(undefined);
    arktest.assertEQ(result, "-10")
}

function testBigIntToString3() {
    let n = new bigint(0b10);
    let result = n.toString(undefined);
    arktest.assertEQ(result, "2")
}

function testBigIntToString4() {
    let n = new bigint(0x10);
    let result = n.toString(undefined);
    arktest.assertEQ(result, "16")
}

function testBigIntToString5() {
    let n = 10n;
    let r: number|undefined = undefined;
    let result = n.toString(r);
    arktest.assertEQ(result, "10")
    r = 16;
    result = n.toString(r);
    arktest.assertEQ(result, "a")
}

function testBigIntToString6() {
    let n = -106233993686329866388788451887415352037722042843148312087371682210961017881854n;
    let result = n.toString(16);
    arktest.assertEQ(result, "-eade4f43ef29faba54e2276cd2fb250b9cfe381912e4cb7b4d98ca527fa544fe")
}

function testBigIntToString7() {
    let n = -24734529127909305130315118443084729844173892131808286553102471449170n;
    let result = n.toString(2);
    arktest.assertEQ(result, "-11101010110111100100111101000011111011110010100111111010101110100101010011100010001001110110110011010010111110110010010100001011100111001111111000111000000110010001001011100100110010110111101101001101100110001100101001010010")
}

function testBigIntToString8() {
    let n = 17994542461121645181648364671329822175764606026173143432826568886866n;
    let result = n.toString(4);
    arktest.assertEQ(result, "2222313210331003323302213322232211103202021312303102332302110023213033320320012101023210302313231031212030221102")
}

function testBigIntToString9() {
    let n = 4581999761941507497992146088338584893584933027760574876539n;
    let result = n.toString(8);
    arktest.assertEQ(result, "5655711720767451765351247042355464575445027163761601442271145573")
}

function testBigIntToString10() {
    let n = 4581999761941507497992146088338584893584933027760574876539n;
    let result = n.toString(32);
    arktest.assertEQ(result, "2tdsjq3tskvleiks8jmpknr4k5ppvho349e9irr")
}

function testBigIntToString11() {
    let n = 4581999761941507497992146088338584893584933027760574876539n;
    let result = n.toString(10);
    arktest.assertEQ(result, "4581999761941507497992146088338584893584933027760574876539")
}

function testBigIntConstructor1() {
    let n = BigInt(0x0102030405060708)
    arktest.assertEQ(n, BigInt("72623859790382848"))

    const errorcheck = (e: Error) => { 
        return (e instanceof RangeError) &&
            e.message === 'the Double value 72623859.79038285 cannot be converted to a BigInt because it is not an integer'
        }
    arktest.expectThrow((): void => { n = BigInt(72623859.790382848) }, errorcheck)
}

function testBigIntConstructor2() {
    let n = BigInt(0x010203040506)
    arktest.assertEQ(n,  BigInt("1108152157446"))
    n = BigInt(72623859)
    arktest.assertEQ(n, BigInt("72623859"))
}

function testBigIntConstructor3() {
    let n = BigInt("72623859790382856")
    arktest.assertEQ(n.toString(), "72623859790382856")
    n = BigInt("72623859790382848")
    arktest.assertEQ(n.toString(), "72623859790382848")
}

function testBigIntConstructor4() {
    let n = BigInt(0.0)
    arktest.assertEQ(n, BigInt("0.0"))
    arktest.assertEQ(n.toString(), "0")
}

function testBigIntFromDoubleInt(): void {
    let x = 1.0;
    let b = new BigInt(x);
    arktest.assertEQ(b, new BigInt(1));
}

function testBigIntFromDoubleFraction(): void {
    let x = 1.2;
    arktest.expectThrow(
        (): void => { new BigInt(x); },
        (e: Error) => e instanceof RangeError
    );
}

function testBigIntFromDoublePosInf(): void {
    let x = Double.POSITIVE_INFINITY;
    arktest.expectThrow(
        (): void => { new BigInt(x); },
        (e: Error) => e instanceof RangeError
    );
}

function testBigIntFromDoubleNegInf(): void {
    let x = Double.NEGATIVE_INFINITY;
    arktest.expectThrow(
        (): void => { new BigInt(x); },
        (e: Error) => e instanceof RangeError
    );
}

function testBigIntFromDoubleNaN(): void {
    let x = Double.NaN;
    arktest.expectThrow(
        (): void => { new BigInt(x); },
        (e: Error) => e instanceof RangeError
    );
}

function testBigIntFromDoubleLarge(): void {
    let x = 1024.0;
    let b = new BigInt(x);
    arktest.assertEQ(b, new BigInt(1024));
}

function testBigIntFromDoubleNegativeExp(): void {
    let x = -4e10;
    let b = new BigInt(x);
    arktest.assertEQ(b, new BigInt(-4_00000_00000));
}

function testBigIntFromDoubleFloat32Underflow(): void {
    let MIN_VAL = -3.4028235e38;
    let arr1 = new Float32Array([MIN_VAL]);
    let bval = BigInt(arr1[0]);
    let arr = new BigInt64Array([bval]);
    arktest.assertTrue(arr[0] == 0);
    arktest.assertEQ(arr[0], BigInt(0));
}

function testBigIntNumberMax(): void {
  let bi1 = 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368n;
  arktest.assertTrue(bi1 == Number.MAX_VALUE);
  arktest.assertFalse(bi1 >  Number.MAX_VALUE);
}


function main(): int {
    const suite = new arktest.ArkTestsuite("test bigint toString by undefined")
    
    suite.addTest("convert 10 to toString(undefined)", testBigIntToString1)
    suite.addTest("convert -10 to toString(undefined)", testBigIntToString2)
    suite.addTest("convert 0b10 to toString(undefined)", testBigIntToString3)
    suite.addTest("convert 0x10 to toString(undefined)", testBigIntToString4)
    suite.addTest("test union number|undefined as params for toString", testBigIntToString5)
    suite.addTest("convert -106233993686329866388788451887415352037722042843148312087371682210961017881854 to toString(16)",testBigIntToString6)
    suite.addTest("convert -24734529127909305130315118443084729844173892131808286553102471449170 to toString(2)",testBigIntToString7)
    suite.addTest("convert 17994542461121645181648364671329822175764606026173143432826568886866 to toString(4)",testBigIntToString8)
    suite.addTest("convert 4581999761941507497992146088338584893584933027760574876539 to toString(8)",testBigIntToString9)
    suite.addTest("convert 4581999761941507497992146088338584893584933027760574876539 to toString(32)",testBigIntToString10)
    suite.addTest("convert 4581999761941507497992146088338584893584933027760574876539 to toString(10)",testBigIntToString11)
    suite.addTest("test large numbers and decimals", testBigIntConstructor1)
    suite.addTest("test ordinary numbers", testBigIntConstructor2)
    suite.addTest("test strings", testBigIntConstructor3)
    suite.addTest("test 0.0", testBigIntConstructor4)
    suite.addTest("bigint from double: integer", testBigIntFromDoubleInt);
    suite.addTest("bigint from double: fractional -> throw", testBigIntFromDoubleFraction);
    suite.addTest("bigint from double: +inf -> throw", testBigIntFromDoublePosInf);
    suite.addTest("bigint from double: -inf -> throw", testBigIntFromDoubleNegInf);
    suite.addTest("bigint from double: NaN -> throw", testBigIntFromDoubleNaN);
    suite.addTest("bigint from double: 1024", testBigIntFromDoubleLarge);
    suite.addTest("bigint from double: -4e10", testBigIntFromDoubleNegativeExp);
    suite.addTest("bigint from double: float32 underflow", testBigIntFromDoubleFloat32Underflow);
    suite.addTest("bigint from number max value", testBigIntNumberMax);
    
    return suite.run()
}
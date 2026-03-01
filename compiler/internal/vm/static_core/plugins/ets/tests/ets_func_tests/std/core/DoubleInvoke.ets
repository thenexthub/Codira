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
function testVoidInvoke(): void {
    let tmp = Number();
    arktest.assertEQ(tmp.toFixed(2).toString(), '0.00');
}

function testStringInvoke(): void {
    const str:String = '123.456';
    let tmp = Number(str);
    arktest.assertEQ(tmp.toFixed(2).toString(), '123.46');
}

function testNumberInvoke(): void {
    const num:Number = 234.567;
    let tmp = Number(num);
    arktest.assertEQ(tmp.toFixed(2).toString(), '234.57');
}

function testBigintInvoke(): void {
    const big:BigInt = 1234567890n;
    let tmp = Number(big);
    arktest.assertEQ(tmp.toFixed(2).toString(), '1234567890.00');
}

function testUndefinedInvoke(): void {
    let tmp = Number(undefined);
    arktest.assertEQ(tmp.toFixed(2).toString(), 'NaN');
}

function testNullInvoke(): void {
    let tmp = Number(null);
    arktest.assertEQ(tmp.toFixed(2).toString(), '0.00');
}

function main() {
    testVoidInvoke();
    testStringInvoke();
    testNumberInvoke();
    testBigintInvoke();
    testUndefinedInvoke();
    testNullInvoke();
}

'use strict';
/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
let buffer = new ArrayBuffer(1);
let dataView = new DataView(buffer);
dataView.setUint8(0, 255);
export let systemHexadecimal = 16;
export let systemBinary = 2;
export let tsInt = 1;
export let tsBigInt = 1n;
export let tsBinary = (256 >> 1).toString(systemBinary);
export let tsHexadecimal = (300).toString(systemHexadecimal);
export let tsIntString = '1';
export let tsByte = dataView;
export let tsFloat = 3.14;
export let tsExponential = tsInt.toExponential();

export let returnBigInt = () => 1000000n;
export let returnBigIntObj = () => BigInt(1);

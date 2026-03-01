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

const buffer = new ArrayBuffer(1);
const dataView = new DataView(buffer);
dataView.setUint8(0, 255);

export const systemHexadecimal = 16;
export const systemBinary = 2;
export const tsInt = 1;
export const tsBigInt = 1n;
export const tsBinary = (256 >> 1).toString(systemBinary);
export const tsHexadecimal = (300).toString(systemHexadecimal);
export const tsIntString = '1';
export const tsByte = dataView;
export const tsFloat = 3.14;
export const tsExponential = tsInt.toExponential();

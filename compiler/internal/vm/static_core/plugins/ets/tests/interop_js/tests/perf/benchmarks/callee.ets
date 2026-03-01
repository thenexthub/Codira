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

class OBJ  {
    a = 1;
    b = '2';
    c: Object = {};
};
let o = new OBJ;

export class STSCallee
{
    static GetObject(): Object {
        return o;
    };
    static GetNumber(): double {
        return 24;
    };
    static GetString(): string {
        return 'abc';
    };
    static GetArray(): FixedArray<double> {
        return [1, 2, 5];
    };
    static PassVoid(): void {};

    static PassStrings (
        u0: string,
        u1: string,
        u2: string,
        u3: string,
        u4: string,
        u5: string,
        u6: string,
        u7: string,
        u8: string,
        u9: string
    ): void {};

    static PassArrays (
        u0: FixedArray<double>,
        u1: FixedArray<double>,
        u2: FixedArray<double>,
        u3: FixedArray<double>,
        u4: FixedArray<double>,
        u5: FixedArray<double>,
        u6: FixedArray<double>,
        u7: FixedArray<double>,
        u8: FixedArray<double>,
        u9: FixedArray<double>
    ): void {};

    static PassNumbers (
        u0: number,
        u1: number,
        u2: number,
        u3: number,
        u4: number,
        u5: number,
        u6: number,
        u7: number,
        u8: number,
        u9: number
    ): void {};
}

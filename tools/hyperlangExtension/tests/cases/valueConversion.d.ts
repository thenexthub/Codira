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
import { MyStringEnum, MyNumericEnum } from './exportAlias';

interface I1 {
    a: number;
    b?: string;
    c: Uint8Array;
    d?: ArrayBuffer;
    e: Float32Array;
    f?: boolean;
    g: Record<string, Uint8Array>;
    h: MyStringEnum;
    i: bigint;
}

class C1 {
    static sa = 123;
    static sb: string;
    static sc?: string = "abc"
    static readonly sd = 999
    static readonly se: string = "zzz"
    va: number = 234;
    vb: Float32Array;
    vc?: MyNumericEnum;
    vd? = "def";
    ve?: bigint[];
}

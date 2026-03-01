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

export class Fields {
    readonly readonlyBoolean_: boolean = true;
    public static staticFloat_: float = 0.0f;
    public number_: number = 2.2;
    public Number_: Number = 3.3;
    public double_: double = 4.4;
    public Double_: Double = 5.5;
    int_: int = 6;
    string_: String = "Panda string!!";
    nullableString_: string | null = "nullable string";
    object_: Object = new Object();
    rangeError_: RangeError = new RangeError();
    uint8Array_: Uint8Array = new Uint8Array();
    uint16Array_: Uint16Array = new Uint16Array();
    uint32Array_: Uint32Array = new Uint32Array();
    public cb = (a: number = 1, b: number = 2, c: number = 3) => {};
};
Fields.staticFloat_ = 1.1f;

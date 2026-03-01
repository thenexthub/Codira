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

import {
    tsNumber,
    increment,
    tsString,
    concatString,
    tsTrue,
    tsFalse,
    changeTrue,
    changeFalse,
    tsUndefined,
    changeUndefined,
    tsNull,
    changeNull,
    tsSymbol,
    changeSymbol,
    tsBigInt,
    changeBigInt
} from 'main_js';

function checkChangeNumber(): boolean {
    const oldValue = tsNumber;
    increment();

    return tsNumber.toInt() > oldValue.toInt();
}

function checkChangeString(): boolean {
    const oldValue = tsString;
    concatString();

    return tsString !== oldValue;
}

function checkChangeTrue(): boolean {
    const oldValue = tsTrue;
    changeTrue();

    return oldValue !== tsTrue;
}

function checkChangeFalse(): boolean {
    changeFalse();

    return tsFalse;
}

function checkChangeUndefined() {
    changeUndefined();

    return tsUndefined === tsNumber;
}

function checkChangeNull() {
    changeNull();

    return tsNull === tsNumber;
}

function checkChangeSymbol(): boolean {
    const oldValue = tsSymbol;
    changeSymbol();

    return oldValue !== tsSymbol;
}

function checkChangeBigInt(): boolean {
    const oldValue = tsBigInt;
    changeBigInt();

    return oldValue !== tsBigInt;
}

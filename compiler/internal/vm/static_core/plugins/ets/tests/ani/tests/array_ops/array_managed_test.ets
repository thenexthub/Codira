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

function getEmptyArray(): Array<Int> {
    let a: Array<Int> = []
    return a
}

function getArray(): Array<Int> {
    let a: Array<Int> = [1, 2, 3, 4, 5]
    return a
}

function checkEmptyArray(a: Array<Int>): boolean {
    return a.length == 0
}

function checkArray(a: Array<Int>): boolean {
    return a[0] == 10 && a[1] == 20 && a[2] == 30 && a[3] == 40 && a[4] == 50
}

class ArrayWithThrowingLength<T> extends Array<T> {
    public override get length(): int {
        throw new Error("length was called")
    }
}

export function getExtendedArray() {
    return new ArrayWithThrowingLength<int>()
}

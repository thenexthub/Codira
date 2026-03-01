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

class ArrayClass {
    public static array: FixedArray<double> = [1.0, 2.0, 3.0, 4.0, 5.0]

    public static changeStaticArray() {
        ArrayClass.array[2] = 22.0
        ArrayClass.array[4] = 44.0
    }

    public static checkStaticArray(): boolean {
        return (
            ArrayClass.array[0].toInt() == 1.0 &&
            ArrayClass.array[1].toInt() == 2.0 &&
            ArrayClass.array[2].toInt() == 22.0 &&
            ArrayClass.array[3].toInt() == 33.0 &&
            ArrayClass.array[4].toInt() == 44.0
        )
    }
}

function getArray(): FixedArray<double> {
    let a: FixedArray<double> = [1.0, 2.0, 3.0, 4.0, 5.0]
    return a
}

function checkArray(a: FixedArray<double>): boolean {
    return a[0].toInt() == 1 && a[1].toInt() == 2 && a[2].toInt() == 30 && a[3].toInt() == 40 && a[4].toInt() == 50
}

function getSpecialArray(): FixedArray<double> {
	let min: double = Number.MIN_VALUE
	let max: double = Number.MAX_VALUE
	let minus: double = -Number.MAX_VALUE
    let a: FixedArray<double> = [min, max, minus, 0, 0, 0]
    return a
}

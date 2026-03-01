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
    public static array: FixedArray<float> = [1.0f, 2.0f, 3.0f, 4.0f, 5.0f]

    public static changeStaticArray() {
        ArrayClass.array[2] = 22.0f
        ArrayClass.array[4] = 44.0f
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

function getArray(): FixedArray<float> {
    let a: FixedArray<float> = [1.0f, 2.0f, 3.0f, 4.0f, 5.0f]
    return a
}

function checkArray(a: FixedArray<float>): boolean {
    return a[0].toInt() == 1 && a[1].toInt() == 2 && a[2].toInt() == 30 && a[3].toInt() == 40 && a[4].toInt() == 50
}

function getSpecialArray(): FixedArray<float> {
	let min: float = new Float(1.40129846432481707e-45)
	let max: float = new Float(3.40282346638528860e+38)
	let minus: float = new Float(-3.40282346638528860e+38)
    let a: FixedArray<float> = [min, max, minus, 0.0f, 0.0f, 0.0f]
    return a
}

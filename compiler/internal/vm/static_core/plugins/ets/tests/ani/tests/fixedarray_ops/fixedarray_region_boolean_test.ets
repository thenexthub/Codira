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
    public static array: FixedArray<boolean> = [true, true, true, true, true]

    public static changeStaticArray() {
        ArrayClass.array[2] = false
        ArrayClass.array[4] = false
    }

    public static checkStaticArray(): boolean {
        return (
            ArrayClass.array[0] == true &&
            ArrayClass.array[1] == true &&
            ArrayClass.array[2] == false &&
            ArrayClass.array[3] == false &&
            ArrayClass.array[4] == false
        )
    }
}

function getArray(): FixedArray<boolean> {
    let a: FixedArray<boolean> = [true, true, true, true, true]
    return a
}

function checkArray(a: FixedArray<boolean>): boolean {
    return a[0] == true && a[1] == true && a[2] == true && a[3] == false && a[4] == true
}

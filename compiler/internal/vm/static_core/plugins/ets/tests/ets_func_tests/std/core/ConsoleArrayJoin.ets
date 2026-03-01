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

function main() {

    let numInt: int = 1
    let strInt = "intStr"
    let numShort: short = 2
    let strShort = "shortStr"
    let numLong: long = 3
    let strLong = "longStr"
    let numFloat: float = 4.0f
    let strFloat = "floatStr"
    let numDouble: double = 5.0
    let strDouble = "doubleStr"
    let num = 1
    let str = "str"

    console.groupCollapsed(num, str)
    console.groupEnd()
    console.groupCollapsed(str)
    console.groupEnd()
    console.groupCollapsed('count %d')
    console.groupEnd()
    console.groupCollapsed()
    console.groupEnd()
    console.groupCollapsed(num, str, "sdsf")
    console.groupEnd()
    console.groupCollapsed(numInt, strInt)
    console.groupEnd()
    console.groupCollapsed(numShort, strShort)
    console.groupEnd()
    console.groupCollapsed(numLong, strLong)
    console.groupEnd()
    console.groupCollapsed(numFloat, strFloat)
    console.groupEnd()
    console.groupCollapsed(numDouble, strDouble)
    console.groupEnd()
}

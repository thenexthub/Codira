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
/* #24986
class A
{
    a(b : Int) : Int
    {
        return b
    }
}

function foo(a : Int) : Int
{
    a = a + 1;
    a = a - 1;
    return a
}

function main()
{
let i: Char = 15 as char
    let i1: Float = 5.0f
    let i2: Double = 5.0
    let i3: Short = 5
    let i4: Long = 5 as long
    let i5: Int = 5
    let i6: Byte = 5 as byte
    let i7: Boolean = true

    let ui: char = i
    let ui1: float = i1
    let ui2: double = i2
    let ui3: short = i3
    let ui4: long = i4
    let ui5: int = i5
    let ui6: byte = i6
    let ui7: boolean = i7




    ui5 = foo(ui5)
    let a_1 : A = new A
    ui5 = a_1.a(ui5)
    console.println(ui5)
    switch(i5)
    {
        case 5:
            break;
        default:
            break;
    }

    let uni1: Int = ~i5

    ui2 = i2
    i2 = ui2

    i5++
    i5--
    i5 += 1
    i5 *= 1
    let ar1: int[] = [1, 2, 3, 4, 5, 6, 7]
    ar1[i5]

    i5 << 1
    i5 as int

    if(i5 > 3)
    i5 & 1
    for(i5; i5 < 10; i5++)
    {

    }

    switch((i5 as int))
    {
        case 5:
            break;
        default:
            break;
    }

    let ni5 : Int = 5 as Int
    let nui5 : int = ni5 as int
}
*/
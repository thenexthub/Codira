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

/*---
desc: >-
    Each predefined value type has a corresponding predefined class type (called boxed type)
    that wraps the value of the predefined value type: Number, Byte, Short, Int, Long,
    Float, Double, Char, and Boolean.
---*/

let b = new Byte()
let s = new Short()
let i = new Int()
let l = new Long()
let f = new Float()
let d = new Double()
let n = new Number()
let c = new Char()
let bool = new Boolean()

function main() {
    arktest.assertEQ( b, 0 )
    arktest.assertEQ( s, 0 )
    arktest.assertEQ( i, 0 )
    arktest.assertEQ( l, 0 )
    arktest.assertEQ( f, 0.0f )
    arktest.assertEQ( d, 0.0 )
    arktest.assertEQ( n, 0.0 )
    arktest.assertEQ( c, c'\u0000' )
    arktest.assertEQ( bool, false )

    arktest.assertEQ( b, 0 )
    arktest.assertEQ( s, 0 )
    arktest.assertEQ( i, 0 )
    arktest.assertEQ( l, 0 )
    arktest.assertEQ( f, 0.0f )
    arktest.assertEQ( d, 0.0 )
    arktest.assertEQ( n, 0.0 )
    arktest.assertEQ( c, c'\u0000' )
    arktest.assertEQ( bool, false )
}

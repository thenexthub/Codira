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
desc: Implementing property from interface. Sample from specification
assert: >-
    If a property is defined in a form that requires a setter, then the implementation of the property
    in the form of a readonly field causes a compile-time error
tags: [compile-only, negative]
---*/

interface Style {
    writable: number
    set color(s: string)
}

class StyleClassTwo implements Style {
    readonly color: string = "" // compile-time error
    readonly writable: number = 0 // compile-time error
}

function write_into_read_only(s: Style) {
    s.color = "Black"
    s.writable = 666
}

function main() {
    write_into_read_only(new StyleClassTwo)
}

/* @@? 30:14 Error TypeError: Cannot implement interface Style: property 'color' must have a setter,
but the implementation is readonly */
/* @@? 31:14 Error TypeError: Cannot implement interface Style: property 'writable' must have a setter, b
ut the implementation is readonly */
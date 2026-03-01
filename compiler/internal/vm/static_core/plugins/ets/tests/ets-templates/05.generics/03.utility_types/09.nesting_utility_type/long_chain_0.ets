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
desc: If more than one utility types are required then they can be nested
tags: []
---*/

interface Issue {
    title?: string
    description?: string
}


type T0 = Required<Readonly<Issue>>
type T1 = Readonly<T0>
type T2 = Required<T1>
type T3 = Readonly<T2>
type T4 = Required<T3>
type T5 = Readonly<T4>
type T6 = Required<T5>
type T7 = Readonly<T6>
type T8 = Required<T7>
type T9 = Readonly<T8>
type TA = Readonly<T9>
type TB = Readonly<TA>
type TC = Readonly<TB>
type TD = Readonly<TC>
type TE = Readonly<TD>
type TF = NonNullable<TE>

function main() {
    const myIssue: TF = { title: 'aa', description: 'bb' }
    arktest.assertEQ('aa', myIssue.title)
}




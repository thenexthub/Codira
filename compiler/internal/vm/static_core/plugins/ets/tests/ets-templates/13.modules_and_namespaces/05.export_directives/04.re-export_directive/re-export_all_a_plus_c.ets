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
desc: Re-export directive, all names should be available
files: ['./modules/module_a.ets', './modules/module_c.ets', './modules/module_all_a_plus_c.ets']
---*/

import { ClassA, InterfaceA, foo, BAR, AOI, ClassC, InterfaceC, baz, BCR, AOI_C } from './modules/module_all_a_plus_c'

class InterfaceAImpl implements InterfaceA, InterfaceC {
}

type ImpAOI = AOI
type ImpAOI_C = AOI_C

function main() {
    foo()
    baz()
    new ClassA
    new ClassC
    new InterfaceAImpl
    arktest.assertEQ(146, BAR)
    arktest.assertEQ(146, BCR)
}

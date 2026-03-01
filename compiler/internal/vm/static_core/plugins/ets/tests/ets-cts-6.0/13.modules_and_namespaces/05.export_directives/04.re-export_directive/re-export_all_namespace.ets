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
desc: Re-export directive
files: ['./modules/module_b_re-export.ets', './modules/module_b.ets']
---*/

import { name } from "./modules/module_b_re-export"

function main() : void
{
    // Export function
    arktest.assertEQ(name.bar(), 100);

    // Export class
    let foo_cls : name.foo = new name.foo();
    arktest.assertEQ(foo_cls.foo_bar(), 101);
    arktest.assertEQ(name.foo.foo_bar_s(), 102);

    // Export interface
    let foo_inter : name.foo_Inter = {fld1: 10, fld2: "Hi"};
    arktest.assertEQ(foo_inter.fld1, 10);
    arktest.assertEQ(foo_inter.fld2, "Hi");

    // Export enum
    arktest.assertEQ(name.Color.Red.valueOf(), 10);
    arktest.assertEQ(name.Color.Blue.valueOf(), 20);

    // Export namespace
    arktest.assertEQ(name.NS.Color.Red.valueOf(), 5);
    arktest.assertEQ(name.NS.Color.Blue.valueOf(), 10);

    // Export variable
    arktest.assertEQ(name.numExported, 111);

    // Export annotation
    @name.MyAnno let anno : int = 10;

    // Export type alias
    let foo_cls1 : name.fooCls = new name.foo();
    arktest.assertEQ(foo_cls1.foo_bar(), 101);
}

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
  files: [declare1.ets]
---*/

import { AttributeModifier, AttributeUpdater } from './declare1.d.ets'

/**
 * not ovveriding in the implementation class of the declaration file (the scenario where an error is found)   
 */
class ButtonAttribute { }

class TestAttributeUpdater extends AttributeUpdater<ButtonAttribute> {
    applyNormalAttribute(instance: ButtonAttribute): void {
        this.prop = 'son_call';
    }
}

function foo(tst: AttributeModifier<ButtonAttribute>) {
    tst.applyNormalAttribute(new ButtonAttribute());
}

let tmp = new TestAttributeUpdater();
foo(tmp);
arktest.assertEQ(tmp.prop, 'son_call');

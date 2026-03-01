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
desc: The upper-layer interface has a method. In multi-layer inheritance, the sub-interface
  overrides all the methods of the upper-layer interface. The return value of the
  method is of the char data type.
name: multi_layer_interface_inheritance_override_ext_9
---*/

interface GrandParentInterface {
  grandparentMethod(): char
}
interface ParentInterface extends GrandParentInterface {
  parentMethod(): char
}
interface ChildInterface extends ParentInterface {
  grandparentMethod(): char {
    return 3;
  }
  parentMethod(): char {
    return 4;
  }
}
class ConcreteClass implements ChildInterface {
}

function main(): void {
  const obj: ChildInterface = new ConcreteClass();
  let tmp1: char = obj.grandparentMethod();
  arktest.assertEQ(tmp1, 3);
  let tmp2: char = obj.parentMethod();
  arktest.assertEQ(tmp2, 4);
}

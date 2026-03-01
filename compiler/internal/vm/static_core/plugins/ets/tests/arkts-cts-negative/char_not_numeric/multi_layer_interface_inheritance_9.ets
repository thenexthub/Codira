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
desc: In multi-layer inheritance, the sub-interface inherits the methods of all upper-layer
  interfaces. The return data type of the method is char.
name: multi_layer_interface_inheritance_9
---*/

interface GrandParentInterface {
  grandparentMethod(): char {
    return 1;
  }
}

interface ParentInterface extends GrandParentInterface {
  parentMethod(): char {
    return 2;
  }
}

interface ChildInterface extends ParentInterface {

}

class ConcreteClass implements ChildInterface {

}

function main(): void {
  const obj: ChildInterface = new ConcreteClass();

  let tmp1: char = obj.grandparentMethod();
  arktest.assertEQ(tmp1, 1);

  let tmp2: char = obj.parentMethod();
  arktest.assertEQ(tmp2, 2);
}

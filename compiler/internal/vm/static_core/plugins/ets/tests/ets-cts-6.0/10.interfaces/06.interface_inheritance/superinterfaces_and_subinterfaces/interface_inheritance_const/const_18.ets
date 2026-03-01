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
desc: Test whether the default value of constants in the subclass interface can be redefined(boolean).
---*/

interface ParentInterface {
  readonly CONSTANT_VALUE: boolean;
}

interface ChildInterface extends ParentInterface {}

class ParentClass implements ParentInterface {
  readonly CONSTANT_VALUE: boolean = true;
}
class ChildClass implements ChildInterface {
  readonly CONSTANT_VALUE: boolean = false;
}

function main(): void {
  let parentInstance = new ParentClass();
  let childInstance = new ChildClass();
  arktest.assertEQ(parentInstance.CONSTANT_VALUE, true);
  arktest.assertEQ(childInstance.CONSTANT_VALUE, false);
}

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
desc: Ensure that a subclass inherits the initial value of a user class type member variable from the parent class.
---*/

class UserClass {
  public name: string = "User";
}

class Base {
  protected value: UserClass;

  constructor() {
    this.value = new UserClass();
  }
}

class Derived extends Base {
  public value1: UserClass;
  constructor() {
    super();
    this.value1 = this.value;
  }
}

function main(): void {
  let instance = new Derived();
  arktest.assertEQ(instance.value1.name, "User", "Subclass should inherit the initial value of the UserClass type member variable");
}
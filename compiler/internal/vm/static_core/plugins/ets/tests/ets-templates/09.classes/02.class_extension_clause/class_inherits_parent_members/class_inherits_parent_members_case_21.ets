/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS " BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Ensure that a subclass inherits the initial value of a boolean[] type member variable from the parent class.
---*/

class Base {
  public value: boolean[] = [true, false, true];
}

class Derived extends Base {}

function main(): void {
  let instance = new Derived();
  arktest.assertEQ(instance.value[0], true, "Subclass should inherit the initial value of the boolean[] type member variable");
  arktest.assertEQ(instance.value[1], false, "Subclass should inherit the initial value of the boolean[] type member variable");
  arktest.assertEQ(instance.value[2], true, "Subclass should inherit the initial value of the boolean[] type member variable");
}
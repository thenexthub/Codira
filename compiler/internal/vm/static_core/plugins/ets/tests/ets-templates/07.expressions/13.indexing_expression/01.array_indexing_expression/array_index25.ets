/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
desc: Test the array index type safety of arrays of parent and child classes.
tags: [compile-only, negative]
---*/

class Animal {}
class Dog extends Animal { name: string }

function main(): void {
  let animalArr: Animal[] = [new Dog(), new Dog()];
  arktest.assertEQ(animalArr[0].name, "Buddy");
}
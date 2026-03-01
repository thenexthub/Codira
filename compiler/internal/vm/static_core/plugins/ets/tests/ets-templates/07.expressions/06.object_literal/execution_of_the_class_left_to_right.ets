/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: Name-value pairs of the class composite are then executed from left to right in the textual order they occur in the source code
---*/

let res: int;

class Person {
  name: String = "";
  age: int;
}
  
function getName(): String {
  res = 1;
  return "Bob";
}

function getAge(): int {
  arktest.assertEQ( res, 1 )
  res = 2;
  return 18;
}

function main(): void {
  res = 0;
  let person: Person = {name: getName(), age: getAge()};

  arktest.assertEQ( res, 2 )
}

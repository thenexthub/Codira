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
desc: Using a class composite as a function parameter
---*/

class Person {
  age: int;
}

let checkVar: int;

function foo(person: Person): void {
  checkVar = person.age;
}
  
function main(): void {
  checkVar = 0;
  arktest.assertEQ( checkVar, 0 );
  for (let i = 0; i < 20; i++) {
    foo({ age: i });
    arktest.assertEQ( checkVar, i );
  }
}
  

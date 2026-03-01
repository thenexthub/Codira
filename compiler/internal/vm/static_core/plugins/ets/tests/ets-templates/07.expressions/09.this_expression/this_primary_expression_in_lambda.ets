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
desc: This reference on object.
assert: The value denoted by this in a lambda body and in the surrounding context is the same.
---*/

type func = (a: A) => int;
class A {  
  public x = 1;
  public foo() {     	
    let bar: func = (a: A): int => { return this.x; };
    return bar(this);
  }
}

function main(): void {
  let instance: A = new A();

  arktest.assertEQ( instance.foo(), 1 );
}

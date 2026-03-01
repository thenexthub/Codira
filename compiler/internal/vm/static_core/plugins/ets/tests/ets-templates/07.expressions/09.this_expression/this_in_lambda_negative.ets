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
desc: Keyword this in lambda.
assert: >-
    It may be used in a lambda expression only if it is allowed in the context in which the lambda expression appears.
tags: [compile-only,negative]
---*/

type func = () => int;
let y = 6;	
class A { 
  bar: func = (): int => { return this.y; };
}

function main(): void{
  let instance: A = new A();
  arktest.assertEQ(instance.bar(), 6 );
}

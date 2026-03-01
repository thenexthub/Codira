/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: Create FixedArray with not covariant function subtypes
tags: [negative, compile-only]
expected_out: "{{c.expected_out}}"
---*/

class A {}
class B extends A {}
class C extends A {}
type HandlerReq = (a: B, b: int) => B
type HandlerCInt = (a: C, b: int) => B
const handlerCInt: HandlerCInt = (a: C, b: int): B => {
  return new B
}
type HandlerBInt = (a: B, b: int) => A
const handlerBInt: HandlerBInt = (a: B, b: int): A => {
  return new B
}
type HandlerB = (a: B) => A
const handlerB: HandlerB = (a: B): A => {
  return new B
}
type HandlerBIntInt = (a: B, b: int, c: int) => B
const handlerBIntInt: HandlerBIntInt = (a: B, b: int, c: int): B => {
  return new B
}
type HandlerNothing = () => C
const handlerNothing: HandlerNothing = (): C => {
  return new C
}

function main(): void {
{{c.logic|indent}}
}

{% endfor %}

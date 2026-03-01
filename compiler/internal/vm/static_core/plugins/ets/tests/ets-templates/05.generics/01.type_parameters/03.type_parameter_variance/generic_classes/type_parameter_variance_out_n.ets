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

{% for c in cases %}

/*---
desc: Test of covariant _out_ type parameter modifier. 
tags: [compile-only, negative]
expected_out: {{c.stdout}}
---*/

class A<out T> {
  readonly field: T = {{c.case.value}} as T
  f(): T { return this.field }
}

function main(): void {
  let arg: {{c.case.type}} = {{c.case.value}}
  let a: A<{{c.case.type}}> = new A<{{c.case.type}}>()
  arktest.assertEQ(a.f(), arg)
}

{% endfor %}

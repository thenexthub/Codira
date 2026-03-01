/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for t in types %}
{% for c in cases %}

/*---
desc: Using custom type of Partial<T>
---*/

{{t}} A {
  i: number {% if t != "interface" %} = 0 {% endif %}
  j: string {% if t != "interface" %} = "" {% endif %}
}

type T = Partial<A>

type R = A

type RP = Partial<R>

function foo(i: Partial<A>) {
  let a: int = 0;
  if(i.i != null) {
    a += 1;
  }
  if(i.j != undefined) {
    a += 2;
  }
  return a;
}

function main() {
  let f: T = {{c.argument}}
  let g: RP = {{c.argument}}
  arktest.assertTrue( foo(f) == {{c.result}} )
  arktest.assertTrue( foo(g) == {{c.result}} )
}

{% endfor %}
{% endfor %}

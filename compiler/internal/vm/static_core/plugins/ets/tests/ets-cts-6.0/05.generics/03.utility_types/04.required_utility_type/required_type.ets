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

{% for prop in properties %}
{% for c in cases %}

/*---
desc: Type Required<T> constructs a type with all properties of T set to required. T must be a class or an interface type.
tags: [{% if c.result == -1 %}compile-only, negative{% endif %}]
---*/

class C {
  {{prop.i}}
  {{prop.j}}
}

function foo(i: Required<C>) {
  let v: int = 0;
  if(i.j != undefined) {
    v += 1;
  }
  if(i.i != 0) {
    v += 2;
  }
  console.log(v)
  return v;
}

function main() {
  arktest.assertTrue( foo( {{c.argument}} ) == {{c.result}} )
}

{% endfor %}
{% endfor %}

/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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

{% for i in default_value %}

/*---
desc: Predefined type declaration.
test_suffix: "{{i.file_name}}"
file_name: "different_file_name"
---*/

class A {
  a: int
}

{% if ":" in i %}
let x = {{i.val}};
function main(): void {
  let y = {{i.val}};
  arktest.assertTrue(x instanceof Function)
  arktest.assertTrue(y instanceof Function)
}
{% else %}
let x: {{i.val}};
function main(): void {
  let y: {{i.val}};
}
{% endif %}

{% endfor %}

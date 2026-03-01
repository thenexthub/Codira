/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

{% for member in members %}
/*---
desc: Check members with internal access modifier in package.
assert: Member or constructor with private modifier can be accessed only within their declaring class.
params: "{{member.body}}"
---*/

package Test;

class D{{loop.index0}} {
  internal {{member.body}}
}

{% if loop.last %}
function main(): void {
    let instance_A: A = new A(2, 3);
    let instanc_A: A = new A(2);
{% for member1 in members %}
    let instance{{loop.index0}}: D{{loop.index0}} = new D{{loop.index0}}();
    instance{{loop.index0}}.{{member1.call}};
{% endfor %}
}
{% endif %}
{% endfor %}

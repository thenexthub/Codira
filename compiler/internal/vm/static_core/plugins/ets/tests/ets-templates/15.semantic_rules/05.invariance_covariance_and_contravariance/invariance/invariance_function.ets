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

{% for f in cases %}

/*---
desc: Invariance refers to the ability to use the originally-specified type as a derived one.
---*/

{{f.customobj}}

function foo( i: {{f.argtype}}): {{f.argtype}} {
  return i;
}

function main() {
  let i:{{f.argtype}} = {{f.input}};
  arktest.assertEQ( foo(i), i );
}

{% endfor %}

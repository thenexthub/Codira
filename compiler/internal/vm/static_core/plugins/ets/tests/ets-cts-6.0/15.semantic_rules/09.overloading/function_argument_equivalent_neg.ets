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

{% for c in cases %}

/*---
desc: >-
    The correctness check for functions overloading is performed if two or more functions with the same name are accessible (see Accessible) in a scope (see Scopes).
tags: [compile-only, negative]
---*/

type TPL = [boolean, string];
type FUN = () => boolean;

function foo({{c.arg}}): void {}
function foo({{c.arg}}): void {}

function main(): void {
  let c: TPL = [true, "some str"];
  foo({{c.call}});
}

{% endfor %}

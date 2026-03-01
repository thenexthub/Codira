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

{% for t in types %}
{% for mode in modes %}
/*---
desc: A compile-time error occurs if a function has at least one parameter or return type of the type FixedArray
   parameterized with a type parameter and function call expression leads to instantiation of the type FixedArray
   with the predefined value type
tags: [compile-only, negative]
---*/

function foo<T>(p: FixedArray<T>): T {
    return p[0]
}

function foo1<T>(p: T): FixedArray<T> {
    // Note new FixedArray<T> would throw CTE per spec 17.2
    let res: FixedArray<T> = [p]
    return res
}

function foo1_caller(): FixedArray<{{t.type}}> {
    let p: {{t.type}} = {{t.init}}
    return foo1<{{t.type}}>(p)
}

function main() {
{% if "param" in mode %}
    let foo_var = foo<{{t.type}}>
    let param: FixedArray<{{t.type}}> = [{{t.init}}]
    let res = foo_var(param)
    arktest.assertTrue(res instanceof {{t.type}})
{% else %}
    let foo_res = foo1_caller()
    arktest.assertTrue(foo_res[0] instanceof {{t.type}})
{% endif %}
}

{% endfor %}
{% endfor %}

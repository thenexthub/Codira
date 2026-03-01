/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
/*---
desc: Verify behavior when a function has at least one parameter or return type of the type FixedArray
   parameterized with a type parameter and function call expression leads to instantiation of the type
   FixedArray with the predefined value type
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

function test_param() {
    let foo_var = foo<{{t.type}}>
    let param: FixedArray<{{t.type}}> = [{{t.init}}]
    let res = foo_var(param)
    arktest.assertTrue(res instanceof {{t.type}}, "expect particular instance in test param")
}

function test_return() {
    let foo_res = foo1_caller()
    arktest.assertTrue(foo_res[0] instanceof {{t.type}}, "expect particular instance in test return")
}

function main() {
    test_param()
    test_return()
}

{% endfor %}

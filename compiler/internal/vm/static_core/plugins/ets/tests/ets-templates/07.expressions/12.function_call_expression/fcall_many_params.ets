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

{%- for c in cases %}
/*---
desc: >-
    A function call expression is used to call a function or a lambda expression.
    Test calls with many parameters.
---*/


let q: boolean|undefined

function foo({{c.decl}} lambda1?: () => void, lambda2?: () => void): boolean {
    return lambda2 == undefined
}

function test_function() {
    arktest.assertTrue(foo({{c.use}} q ? undefined : ()=>{}, undefined))
}

function test_lambda() {
    let test_lambda = ({{c.decl}} lambda1?: () => void, lambda2?: () => void) => { return lambda2 == undefined; }
    arktest.assertTrue(test_lambda({{c.use}} q ? undefined : ()=>{}, undefined))
}

class A {
    foo({{c.decl}} lambda1?: () => void, lambda2?: () => void): boolean {
        return lambda2 == undefined
    }
}

function test_method() {
    arktest.assertTrue(new A().foo({{c.use}} q ? undefined : ()=>{}, undefined))
}

class B {
    static foo({{c.decl}} lambda1?: () => void, lambda2?: () => void): boolean {
        return lambda2 == undefined
    }
}

function test_static_method() {
    arktest.assertTrue(B.foo({{c.use}} q ? undefined : ()=>{}, undefined))
}

class C {
    result: boolean

    constructor({{c.decl}} lambda1?: () => void, lambda2?: () => void) {
        this.result = lambda2 == undefined
    }
}

function test_constructor() {
    arktest.assertTrue(new C({{c.use}} q ? undefined : ()=>{}, undefined).result)
}

function main() {
    test_function()
    test_lambda()
    test_method()
    test_static_method()
    test_constructor()
}
{%- endfor %}

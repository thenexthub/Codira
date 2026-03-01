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
    Entity is considered accessible if it belongs to the current scope (see Scopes) and means that its name can be used
    for different purposes as follows -
    - Type name is used to declare constants
    - Function or method name is used to call the function or method
---*/

enum Colors {Red, Blue=-2}
type TPL = [number, string]

function foo(i: byte) {
    return 1;
}

function foo(i: short) {
    return 2;
}

function foo(i: int) {
    return 3;
}

function foo(i: long) {
    return 4;
}

function foo(i: float) {
    return 5;
}

function foo(i: number) {
    return 6;
}

function foo(i: char) {
    return 7;
}

function foo(i: string) {
    return 8;
}

function foo(i: FixedArray<int>) {
    return 21;
}

function foo(i: FixedArray<FixedArray<Int>>) {
    return 22;
}

function foo(i: TPL) {
    return 24;
}

function foo(i: Object) {
    return -1;
}

{{c.decl}}

{{c.check}}

{% endfor %}

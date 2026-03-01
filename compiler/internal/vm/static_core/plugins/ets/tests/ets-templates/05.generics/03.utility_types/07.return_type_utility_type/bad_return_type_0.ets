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

/*---
desc: ReturnType Utility Type
tags: [compile-only, negative]
---*/

interface IMyInterface {
}

class CMyClass {
}

type TSI = String | Int
type ABU = 'aaaa' | 'bbbb'
type TTL = [int, int, String]
enum Color {Red, Green, Blue}

type RetType = ReturnType<{{t}}>
type RetTypeOrNull = ReturnType<{{t}}> | null

function mtr0(): ReturnType<{{t}}> | null {
    return null
}

function mtr1(): RetTypeOrNull {
    return null
}

function mti0(input: ReturnType<{{t}}>): void {
}

function mti1(input: RetTypeOrNull): void {
}

function main() {
    let x: ReturnType<{{t}}>
    let y: RetType

}

{% endfor %}

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

/*---
desc: Spec example 1
tags: [compile-only]
---*/

interface Base {
    foo (p: number): void // 1st overload signature
    foo (p: string): void // 2nd overload signature
}

interface Derive extends Base {
    foo(p: number|string): void // 1st overload signature overrides both foo from Base
    foo (p: boolean): void // 2nd overload signature
}


function demo(value: Derive) {
    value.foo(1)
    value.foo('1')
    value.foo(true)
}


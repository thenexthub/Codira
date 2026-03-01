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
desc: Ambient Function declarations
tags: [compile-only, not-a-test]
---*/

declare function fun(x: int): string

declare function foo(x: number, y: string, z: Object[]): double

declare function bar<U, V, R>(x: U, y: V): R

declare function baz(y: string, x?: int): string

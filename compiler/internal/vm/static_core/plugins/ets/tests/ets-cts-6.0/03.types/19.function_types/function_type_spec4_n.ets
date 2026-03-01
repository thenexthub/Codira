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
desc: Example for type function
tags: [negative, compile-only]
---*/

type FuncTypeWithOptionalParameters = (x?: number, y?: string) => void
let foo: FuncTypeWithOptionalParameters = (): void => {} // OK: as arguments are just ignored

foo = (p: number):void => {} // CTE as call with zero arguments is invalid
foo = (p1: number, p2?: string): void => {} // Compile-time error: as call with zero arguments is invalid

// compile-time error: no mandatory parameter can follow an optional parameter
type IncorrectFuncTypeWithOptionalParameters = (x?: number, y: string) => void

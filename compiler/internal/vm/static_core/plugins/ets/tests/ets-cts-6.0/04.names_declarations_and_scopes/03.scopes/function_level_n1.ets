/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: >-
    The scope of a name declared immediately within the body of a function
    (method) declaration is the body of that function declaration (method
    or function scope) from the place of declaration and up to the end of
    the body.
tags: [compile-only, negative]
---*/

function f1() {
    let x = 9
}

function main() {
    f1()
    arktest.assertEQ( x, 9 ) // CTE
}

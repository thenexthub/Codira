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
desc: check division by zero for long integer operand
---*/

function divide(a: long, b: long): long {
    return a / b
}

function main(): void {
    const a: long = (() : long => 1)()
    const b: long = (() : long => 0)()
    arktest.assertTrue(a instanceof Long)
    arktest.assertTrue(b instanceof Long)

    let checkException = (e: Error): boolean => { return e instanceof ArithmeticError }
    
    arktest.expectThrow(() => {
        divide(a, b)
    }, checkException)
}
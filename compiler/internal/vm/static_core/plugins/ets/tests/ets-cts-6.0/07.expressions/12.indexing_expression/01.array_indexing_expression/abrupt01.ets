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
    For an array, if the index expression value is less than zero, greater
    than or equal to the array's length, then ArrayIndexOutOfBoundsError is thrown.
---*/


let arr: number[] = [1, 2, 3]

function arrRef(exc: boolean): number[] {
    if (exc) {
        throw new Error()
    }
    return arr
}


function foo(indexingExpressionEvaluated: boolean): void {
	arktest.assertTrue( arrRef(true)[(():int => { indexingExpressionEvaluated = true; return 2 })()] == 3 )
}

function main() {
	let indexingExpressionEvaluated: boolean = false
	let checkException = (e: Error): boolean => { return e instanceof Error }
	arktest.expectThrow(() => { arktest.assertTrue( arrRef(true)[(():int => { indexingExpressionEvaluated = true; return 2 })()] == 3 )}, checkException)
	arktest.assertFalse(indexingExpressionEvaluated)
}

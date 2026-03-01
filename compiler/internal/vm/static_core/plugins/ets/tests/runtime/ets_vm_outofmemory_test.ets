/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

package PreallocTest;

function test(block: int): boolean {
    let storage = new Array<Uint8Array>();
    try {
        const step:long = 1_000_000
        
        for (let i = 0; i < step; i++) {
            let arr = new Uint8Array(block)
            storage.push(arr);
        }
    } catch (e) {
        arktest.assertTrue(e instanceof OutOfMemoryError)
        return true
    }
    return false
}

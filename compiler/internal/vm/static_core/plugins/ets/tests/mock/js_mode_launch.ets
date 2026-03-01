/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

let counter = 0
let addCoroutineId: long = 0;

class JSCoroutine {
    public static native getCoroutineId(): long
}

async function add(a: int): Promise<Object> {
    addCoroutineId = JSCoroutine.getCoroutineId();
    counter += a;
    return new Object();
}

export function main(): int {
    let mainCoroutineId: long = JSCoroutine.getCoroutineId();
    add(5);
    if (counter != 5) {
        console.print("Invalid counter: expected 5 but was " + counter + "\n");
        return 1;
    }
    if (mainCoroutineId == addCoroutineId) {
        console.print("Function add is called in the same coroutine");
        return 2;
    }
    return 0;
}

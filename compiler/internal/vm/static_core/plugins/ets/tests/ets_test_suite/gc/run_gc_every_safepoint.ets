/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

function simpleTestCall(): void {
    let obj_array: FixedArray<Object> = new FixedArray<Object>(4);
    for (let i = 0; i < 4; i++) {
        obj_array[i] = new Object();
        arktest.assertLT(0, GC.getObjectAddress(obj_array[i]), "getObjectAddress for regular object must return value > 0");
    }
    for (let i = 0; i < 4; i++) {
        arktest.assertLT(0, GC.getObjectAddress(obj_array[i]), "getObjectAddress for regular object must return value > 0");
    }
}

function main(): int {
    let obj = new Object();
    arktest.assertLT(0, GC.getObjectAddress(obj), "getObjectAddress for regular object must return value > 0");
    arktest.assertLT(0, GC.getObjectAddress("non-movable string"), "getObjectAddress for non-movable must return value > 0");
    let h_obj: FixedArray<long> = new FixedArray<long>(1024 * 1024);
    simpleTestCall()
    return 0;
}

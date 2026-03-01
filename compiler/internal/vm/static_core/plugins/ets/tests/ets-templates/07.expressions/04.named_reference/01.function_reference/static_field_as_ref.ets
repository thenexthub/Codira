/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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


 /*---
 desc: A function reference refers to a declared or imported function.
       A compile-time error occurs if an overload alias is used in a named reference
 tags: [compile-only, negative]
 ---*/

function fooE(): void { console.log('I am fooE') }

class Holder {
    static ref: Function
}

function main() {
    Holder.ref = fooE
    Holder.ref()
}

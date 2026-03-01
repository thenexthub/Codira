/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
desc: 09.classes/constructor-inheritance
tags: []
---*/

 type NullableString = String|null;

 class Base {
    name: NullableString;
    constructor(param: NullableString) {
        this.name = param;
    }
 }

 class Derive extends Base {}

 function main(): void {
    let b = new Derive();
    arktest.assertEQ( b.name, null)

    b = new Derive("Hello");
    arktest.assertEQ( b.name, "Hello")
 }

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
desc: Access to Common Union Members.
---*/

class A {
    tmp = 1;
    tmp1 = "aa";
    foo() {}
    goo(tmp: number):number {
        return tmp;
    }
}

class B {
    tmp = 2;
    tmp1 = 3.14;
    foo() {}
    goo(tmp: number):number {
        return tmp;
    }
}

function main(): void {
    let account: A | B = new A();
    let result = account.goo(1);
    arktest.assertEQ(result, 1);
}

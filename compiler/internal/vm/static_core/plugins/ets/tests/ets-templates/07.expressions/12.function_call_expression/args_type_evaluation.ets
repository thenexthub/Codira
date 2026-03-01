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
 desc: Function call expression
 ---*/

 function eval0() { return 'Hello World' }

 function eval1() { return 100 }

 function eval2() { return true }

 function consumer1(s: Object, i: Object, b: Object) {
     arktest.assertTrue(s instanceof String)
     arktest.assertTrue(i instanceof int)
     arktest.assertTrue(b instanceof Boolean)
 }

 function consumer2(b: Object) {
     arktest.assertTrue(b instanceof Function0)
 }

 function main(): void {
     consumer1(eval0(), eval1(), eval2())
     consumer2() {
        // Trailing lambda declared there
     }; {
        //But this is an empty block
     }
 }

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
 desc: Compilation no error when attempt to init fields out of order
 ---*/

class Ordered {
    one: String = "";
    two: String = "";
    three: String = "";
    four: String = "";
}

 function main(): void {
    let a: Ordered = { four: "four",  three: "three", two: "two", one: "one"}

    arktest.assertEQ<String, String>(a.one, "one")
    arktest.assertEQ<String, String>(a.two, "two")
    arktest.assertEQ<String, String>(a.three, "three")
    arktest.assertEQ<String, String>(a.four, "four")
 }


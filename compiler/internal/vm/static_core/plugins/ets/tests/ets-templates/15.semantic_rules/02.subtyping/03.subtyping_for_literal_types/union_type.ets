/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Subtype relation between type literal and type union
---*/

type StringUnion = 'some_string' | 'some_other_string'
type StrLiteral = 'some_string'

function foo(a: StrLiteral, b: StringUnion) {
    b = a
    arktest.assertTrue(b instanceof String)
}
function main() {
    foo('some_string', 'some_string')
}



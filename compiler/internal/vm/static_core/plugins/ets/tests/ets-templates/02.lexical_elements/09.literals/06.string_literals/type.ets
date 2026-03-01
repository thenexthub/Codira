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
desc: String literals are of type string, which is a predefined reference type.
---*/

function main() {
    let s: [string, int][] = [ ["", 0], ['', 0], ["\x00", 1], ['\u0000', 1], ["\u{CAFE}", 1], ["\uBABE", 1], ['生日快乐!', 5], ["С Днём рождения!", 16] ]

	arktest.assertEQ(s.length, 8)
    for (let i = 0; i < s.length; i++) {
        arktest.assertTrue( s[i][0] instanceof string , s[i][0] + " is not a string" )
        arktest.assertEQ(s[i][0].length, s[i][1])
    }
}

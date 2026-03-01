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
desc: String with code points
---*/

function main() {
    const s = "\u{c}\u{7545}\u{ea7e}\u{15fb7}\u{1d4f0}\u{24a29}\u{2bf62}\u{3349b}\u{3a9d4}\u{41f0d}\u{49446}\u{5097f}\u{57eb8}\u{5f3f1}\u{6692a}\u{6de63}\u{7539c}\u{7c8d5}\u{83e0e}\u{8b347}\u{92880}\u{99db9}\u{a12f2}\u{a882b}\u{afd64}\u{b729d}\u{be7d6}\u{c5d0f}\u{cd248}\u{d4781}\u{dbcba}\u{e31f3}\u{ea72c}\u{f1c65}\u{f919e}\u{1006d7}\u{107c10}\u{10f149}"

    let res = ""
    let len: int = 0
    for (let i = 12; i < 0x10FFFF; i += 30009) {
        if (Char.codeUnitsToEncode(i) == 1) {
            res += String.fromCharCode(Int.toDouble(i))
            len++
        } else {
            res += String.fromCharCode(Char.toDouble(Char.getHighSurrogate(i)))
            res += String.fromCharCode(Char.toDouble(Char.getLowSurrogate(i)))
            len += 2
        }
    }
    arktest.assertEQ(s, res)
    arktest.assertEQ(s.length, len)
}

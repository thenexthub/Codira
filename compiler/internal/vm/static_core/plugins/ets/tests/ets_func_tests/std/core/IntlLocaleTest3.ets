/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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


let pairs: string[][] = [
["o\u0308", "ö"],
["ä\u0323", "a\u0323\u0308"],
["a\u0308\u0323", "a\u0323\u0308"],
["ạ\u0308", "a\u0323\u0308"],
["ä\u0306", "a\u0308\u0306"],
["ă\u0308", "a\u0306\u0308"],
["\u1111\u1171\u11B6", "퓛"],
["Å", "Å"],
["Å", "A\u030A"],
["x\u031B\u0323", "x\u0323\u031B"],
["ự", "ụ\u031B"],
["ự", "u\u031B\u0323"],
["ự", "ư\u0323"],
["ự", "u\u0323\u031B"],
["Ç", "C\u0327"],
["q\u0307\u0323", "q\u0323\u0307"],
["가", "\u1100\u1161"],
["Å", "A\u030A"],
["Ω", "Ω"],
["Å", "A\u030A"],
["ô", "o\u0302"],
["ṩ", "s\u0323\u0307"],
["ḋ\u0323", "d\u0323\u0307"],
["ḋ\u0323", "ḍ\u0307"],
["q\u0307\u0323", "q\u0323\u0307"],
 ["\uD834\uDD5E", "\uD834\uDD57\uD834\uDD65"],
 ["\uD87E\uDC2B", "北"]
];


function testLocaleCompare() {
    for (let i = 0; i < pairs.length; i++) {
        let pair = pairs[i];
        arktest.assertEQ(pair[0].localeCompare(pair[1]), 0)
    }
}
function main(){
    const suite = new arktest.ArkTestsuite("IntlLocaleTest3 tests")

    suite.addTest("testLocaleCompare", testLocaleCompare)

    return suite.run()

}

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
desc: String literals consist of zero or more characters enclosed between single or double quotes.
---*/

function main() {
    arktest.assertEQ("A\u{00000000000000061}", "Aa")
    arktest.assertEQ("\x61{\u{1D306}\r}", "a{𝌆\x0D}")
    arktest.assertEQ("\ud834\udf06", "𝌆")
    arktest.assertEQ("\\\\\uFFFFFFFF".length, 7)
    arktest.assertEQ("a\0b".length, 3)

    // string with code points
    let s = "\u{0}\u{7537}\u{ea6e}\u{15fa5}\u{1d4dc}\u{24a13}\u{2bf4a}\u{33481}\u{3a9b8}\u{41eef}\u{49426}\u{5095d}\u{57e94}\u{5f3cb}\u{66902}\u{6de39}\u{75370}\u{7c8a7}\u{83dde}\u{8b315}\u{9284c}\u{99d83}\u{a12ba}\u{a87f1}\u{afd28}\u{b725f}\u{be796}\u{c5ccd}\u{cd204}\u{d473b}\u{dbc72}\u{e31a9}\u{ea6e0}\u{f1c17}\u{f914e}\u{100685}\u{107bbc}\u{10f0f3}"
    // same string with hex unicode
    let v = "\u0000\u7537\uea6e\ud817\udfa5\ud835\udcdc\ud852\ude13\ud86f\udf4a\ud88d\udc81\ud8aa\uddb8\ud8c7\udeef\ud8e5\udc26\ud902\udd5d\ud91f\ude94\ud93c\udfcb\ud95a\udd02\ud977\ude39\ud994\udf70\ud9b2\udca7\ud9cf\uddde\ud9ec\udf15\uda0a\udc4c\uda27\udd83\uda44\udeba\uda61\udff1\uda7f\udd28\uda9c\ude5f\udab9\udf96\udad7\udccd\udaf4\ude04\udb11\udf3b\udb2f\udc72\udb4c\udda9\udb69\udee0\udb87\udc17\udba4\udd4e\udbc1\ude85\udbde\udfbc\udbfc\udcf3"
    arktest.assertEQ(s, v)
}

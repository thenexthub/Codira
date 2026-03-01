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
desc: An escape sequence always starts with the backslash character \
---*/

function main() {
    let s1 = '\"\'\b\f\n\r\t\v\\\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ '  // single-quoted
    let s2 = "\"\'\b\f\n\r\t\v\\\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ "  // double-quoted
    let q1 = '\u0022\u0027\u0008\u000c\u000a\u000d\u0009\u000b\u005c\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ ' // single-quoted
    let q2 = "\u0022\u0027\u0008\u000c\u000a\u000d\u0009\u000b\u005c\A\B\C\d\+\=\-\(\)\#\$\@\!\~\`\<\>\/\?\*\{\}\[\]\|\;\:\%\^\&\ю\Я\ " // double-quoted
    arktest.assertTrue(s1 == s2 && s1 == q1 && s1 == q2)
	arktest.assertEQ(s1.length, 42)
}

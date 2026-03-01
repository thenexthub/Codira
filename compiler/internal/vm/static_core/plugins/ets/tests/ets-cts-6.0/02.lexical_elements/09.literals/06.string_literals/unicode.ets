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
desc: String with unicode-formatted characters
---*/

function main() {
    let s = "\u0000\u03e9\u07d2\u0bbb\u0fa4\u138d\u1776\u1b5f\u1f48\u2331\u271a\u2b03\u2eec\u32d5\u36be\u3aa7\u3e90\u4279\u4662\u4a4b\u4e34\u521d\u5606\u59ef\u5dd8\u61c1\u65aa\u6993\u6d7c\u7165\u754e\u7937\u7d20\u8109\u84f2\u88db\u8cc4\u90ad\u9496\u987f\u9c68\ua051\ua43a\ua823\uac0c\uaff5\ub3de\ub7c7\ubbb0\ubf99\uc382\uc76b\ucb54\ucf3d\ud326\ud70f\udaf8\udee1\ue2ca\ue6b3\uea9c\uee85\uf26e\uf657\ufa40\ufe29"

    let j = 0;
    for (let i = 0; i < s.length; i++) {
        arktest.assertTrue( s.charAt(i) == j )
        j += 1001
    }
    arktest.assertEQ(s.length, 66)
}

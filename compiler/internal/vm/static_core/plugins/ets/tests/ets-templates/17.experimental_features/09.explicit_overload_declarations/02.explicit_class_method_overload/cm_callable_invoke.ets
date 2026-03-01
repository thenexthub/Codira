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

{%- for c in cases %}
/*---
desc: >-
    Methods with special names (see Indexable Types, Iterable Types, and Callable Types) can be overloaded like ordinary methods.
    Callable Types: $_invoke
---*/

class G1 {
    static greetI(i: int): string { return 'Hi ' + i + '!' }
    static greetS(s: string): string { return 'Hey ' + s + '!' }
    static overload $_invoke { greetI, greetS }
}

class G2 extends G1 {
    static greetB(b: boolean): string { return b ? 'Hi!' : 'Bye!' }
    static overload $_invoke { greetI, greetS, greetB }
}

function main() {
    arktest.assertEQ('{{c.value}}', {{c.call}});
}
{%- endfor %}

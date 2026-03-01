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
    Callable Types: $_instantiate
---*/

class G1 {
    greeting = "";

    static greetI(factory: () => G1, i: int): G1 {
        let g = factory();
        g.greeting = 'Hi ' + i + '!';
        return g;
    }
    static greetS(factory: () => G1, s: string): G1 {
        let g = factory();
        g.greeting = 'Hey ' + s + '!';
        return g;
    }
    static overload $_instantiate { greetI, greetS }
}

class G2 extends G1 {
    static greetB(factory: () => G2, b: boolean): G2 {
        let g = factory();
        g.greeting = b ? 'Hi!' : 'Bye!';
        return g;
    }
    static overload $_instantiate { greetI, greetS, greetB }
}

function main() {
    let g = {{c.call}};
    arktest.assertEQ('{{c.value}}', g.greeting);
}
{%- endfor %}

/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: Check incorrect overload code. Class constructor overload
tags: [compile-only, negative]
---*/

final class FuncTest {
    public str: string = '';

    constructor() {
        this.str = '100';
    }

    constructor(x: int) {
        this.str = 'int: ' +  x;
    }

    constructor(x: {{c.type}}) {
        this.str = '{{c.type}}: ' + x;
    }
}

function main(): void {
    let ft: FuncTest = new FuncTest({{c.callValue}});
    arktest.assertEQ('{{c.res}}', ft.str);
}

{% endfor %}

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
desc: |-
    A nullish-coalescing expression checks whether the evaluation of the left-hand-side expression equals the nullish
    value.
    Verify proper type resolution in some complicated scenarios.
---*/

{{c.decl}}

interface Options {
    menuList?: {{c.type}}
}

function hee(opt: Options): void {
    let menuList: {{c.type}} = opt.menuList ?? {{c.right_hand_value}}
}

class A implements Options {
    menuList?: {{c.type}} = {{c.init}}
}

class B implements Options {
    menuList?: {{c.type}} = undefined
}

function main(): void {
    hee(new A)
    hee(new B)
}

{%- endfor %}

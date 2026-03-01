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
       value:
       • If so, then the right-hand-side expression evaluation is the result of a nullish-coalescing expression.
       • If not so, then the result of the left-hand-side expression evaluation is the result of a nullish-coalescing expres-
       sion, and the right-hand-side expression is not evaluated (the operator ‘??’ is thus lazy).
       The type of a nullish-coalescing expression is a normalized union type (see Union Types) formed from the following:
           • Non-nullish variant of the type of the left-hand-side expression; and
           • Type of the right-hand-side expression.
       {{c.desc}}
 ---*/
 
 {{c.decl}}

 function main(): void {
     let lhs: {{c.lhs_decl}}
     let rhs: {{c.rhs_decl}}
     let x = lhs ?? rhs
     let x$ = lhs
     if (x$ == null) {
        arktest.assertEQ(x, rhs)
     }
     else {
        arktest.assertEQ(x, x$!)
     }
     arktest.assertEQ(x, {{c.expected_value}})

     // check that x is exactly of expected type, without extra unions
     let y: {{c.expected_type}} = x
 }
 {%- endfor %}
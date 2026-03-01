/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

{% for c in cases %}
/*---
desc: Any call to a function, method, constructor, or lambda with a rest parameter implies that a new array or tuple is created from the arguments provided.
---*/

// function
let arg_fn: {{c.arg_type}} = {{c.arg_value}}
function foo(...rest_parameter: {{c.arg_type}}) {
    rest_parameter[0] = 1234
}
foo(...arg_fn)
arktest.assertEQ(arg_fn[0], 1, `The function call with a rest parameters modified the original {{c.type_hint}} provided in argument`)

// method
let arg_mtd: {{c.arg_type}} = {{c.arg_value}}
class A {
    static foo(...rest_parameter: {{c.arg_type}}): void {
        rest_parameter[0] = 1234
    }
}
A.foo(...arg_mtd)
arktest.assertEQ(arg_mtd[0], 1, `The method call with a rest parameters modified the original {{c.type_hint}} provided in argument`)

// constructor
let arg_ctr: {{c.arg_type}} = {{c.arg_value}}
class B {
    constructor(...rest_parameter: {{c.arg_type}}) {
        rest_parameter[0] = 1234
    }
}
let a = new B(...arg_ctr)
arktest.assertEQ(arg_ctr[0], 1, `The class constructor call with a rest parameters modified the original {{c.type_hint}} provided in argument`)

// lambda
let argument4: {{c.arg_type}} = {{c.arg_value}};
(...rest_parameter: {{c.arg_type}}) => {rest_parameter[0] = 1234}(...argument4);
arktest.assertEQ(argument4[0], 1, `The lambda function call with a rest parameters modified the original {{c.type_hint}} provided in argument`)

{% endfor %}

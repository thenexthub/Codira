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
{% for ctx in context %}

/*---
desc: Specification examples for semantic checks that must be performed to arguments when checking the validity
      of any function, method, constructor, or lambda call.
---*/

{% if "constructor" in ctx %}
// Note limited visibility because of #28527
class A {
    constructor{{ c.decl_args }} {
        {{ c.verify_args|indent(8) }}
    }
}
{%- endif %}

class B {
    bar{{ c.decl_args }}: void {
        {{ c.verify_args|indent(8) }}
    }
}

function foo{{ c.decl_args }}: void {
    {{ c.verify_args|indent(4) }}
}

type my_lambda = {{ c.decl_args }} => void

interface I {
    bar{{ c.decl_args }}: void {
        {{ c.verify_args|indent(8) }}
    }
}

function main() {

{%- if "lambda" in ctx %}
    let my_lambda_var: my_lambda = {{ c.decl_args }}: void => {
        {{ c.verify_args|indent(8) }}
    }
    my_lambda_var{{c.use_args}}
{% endif %}

{%- if "function" in ctx %}
    foo{{c.use_args}}
{% endif %}

{%- if "constructor" in ctx %}
    let a = new A{{c.use_args}}
{% endif %}

{%- if "method" in ctx %}
    new B().bar{{c.use_args}}
{% endif %}

{%- if "interface" in ctx %}
    let i: I = {}
    i.bar{{c.use_args}}
{% endif %}
}

{% endfor %}
{% endfor %}

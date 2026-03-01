/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: No overload for the imported functions with the same name
files: [./modules/function_export_a_{{c.name}}.ets, ./modules/function_export_b_{{c.name}}.ets]
tags: [compile-only, negative]
---*/

import {A} from "./modules/function_export_a_custom.ets"

import {foo} from "./modules/function_export_a_{{c.name}}.ets"
import {foo} from "./modules/function_export_b_{{c.name}}.ets"

function main(): void {
    {{c.call}}
}

{% endfor %}

/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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

{% for key, value in tests | dictsort %}
{% set files = value.files|join(', ') %}
{% set files = files|replace("[loop.index0]", loop.index0) %}
/*---
desc: >-
    The ArkTS standard library (see Standard Library) provides a top-level
    function initModule() with one param-eter of string type. A call to this
    function ensures that the module referred by the argument is available
    and its initialization (see Static Initialization) is performed.
files: [{{files}}]
---*/

{% set call = value.call|replace("[loop.index0]", loop.index0) %}

{{value.decl|replace("[loop.index0]", loop.index0)}}

function main(): void {
    {{call|indent}}
}

{% endfor %}

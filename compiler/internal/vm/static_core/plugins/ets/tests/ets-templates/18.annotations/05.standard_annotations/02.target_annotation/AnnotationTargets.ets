/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

{% for anno in cases %}
{% for atar in targets %}
{% set retention = anno.retention | default('BYTECODE') %}
{% set target_decl = atar.decl | default('NONE') %}

/*---
desc: Target is a standard meta-annotation that is used to annotate a declaration of another annotation.
tags: [compile-only,
{% if anno.decl != target_decl %}
negative
{% endif %}]
---*/

class Dummy {}

{% if retention != 'BYTECODE' %}
@Retention('{{retention}}')
{% endif -%}
@Target([AnnotationTargets.{{anno.decl}}]) @interface MyAnnotation {}

{{atar.target}}

function main() {
}

{% endfor %}
{% endfor %}

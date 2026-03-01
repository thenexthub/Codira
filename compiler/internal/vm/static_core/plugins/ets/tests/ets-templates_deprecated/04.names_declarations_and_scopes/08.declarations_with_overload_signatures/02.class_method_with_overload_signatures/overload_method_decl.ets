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

{% for access_modifier in global_tests_data.access_modifiers %}
{% for method_modifier in global_tests_data.method_modifiers %}
{% for test_data in tests_data %}
{% if not test_data.overload_method_data.return_type in global_tests_data.overload_method_return_types %}
{{ None["ERROR: incorrect value for test_data.overload_method_data.return_type"][0] }}
{% endif %}
{% if not (test_data.overload_method_data.return_type == "mixed" and (("private" in access_modifier) or ("protected" in access_modifier))) %}
/*---
desc: >-
    The syntax of a method with overload signatures is presented below (see also Method Declarations):
    classMethodWithOverloadSignatures:
        methodOverloadSignature*
        classMethodDeclaration
    methodOverloadSignature:
        methodModifier* identifier signature;
---*/

{% if "static" in method_modifier %}
    {% set overload_method_data = test_data.overload_method_data.static %}
{% else %}
    {% set overload_method_data = test_data.overload_method_data.default %}
{% endif %}
{% if access_modifier == "" %}
    {% set overload_method_data = overload_method_data|replace("[access_modifier] ", "") %}
{% endif %}
{% set overload_method_data = overload_method_data|replace("[access_modifier]", access_modifier) %}
{% if method_modifier == "" %}
    {% set overload_method_data = overload_method_data|replace("[method_modifier] ", "") %}
{% endif %}
{% set overload_method_data = overload_method_data|replace("[method_modifier]", method_modifier) %}

{% if "private" in access_modifier %}
    {% if "static" in method_modifier %}
        {% if test_data.overload_method_data.return_type == "void" %}
            {% set modifiers_test_class_data = global_tests_data.modifiers_test_class_data.return_type_void.private_static %}
        {% else %}
            {% set modifiers_test_class_data = global_tests_data.modifiers_test_class_data.return_type_any.private_static %}
        {% endif %}
    {% else %}
        {% if test_data.overload_method_data.return_type == "void" %}
            {% set modifiers_test_class_data = global_tests_data.modifiers_test_class_data.return_type_void.private %}
        {% else %}
            {% set modifiers_test_class_data = global_tests_data.modifiers_test_class_data.return_type_any.private %}
        {% endif %}
    {% endif %}
{% else %}
    {% set modifiers_test_class_data = "" %}
{% endif %}

{% if "protected" in access_modifier %}
    {% if "static" in method_modifier %}
        {% if test_data.overload_method_data.return_type == "void" %}
            {% set modifiers_decl_data = global_tests_data.modifiers_decl_data.return_type_void.protected_static %}
        {% else %}
            {% set modifiers_decl_data = global_tests_data.modifiers_decl_data.return_type_any.protected_static %}
        {% endif %}
    {% else %}
        {% if test_data.overload_method_data.return_type == "void" %}
            {% set modifiers_decl_data = global_tests_data.modifiers_decl_data.return_type_void.protected %}
        {% else %}
            {% set modifiers_decl_data = global_tests_data.modifiers_decl_data.return_type_any.protected %}
        {% endif %}
    {% endif %}
{% else %}
    {% set modifiers_decl_data = "" %}
{% endif %}
{% set modifiers_decl_data = modifiers_decl_data|replace("[class_type_parameters]", test_data.class_type_parameters) %}

{% if "protected" in access_modifier %}
    {% set call_class = global_tests_data.call_class.protected %}
{% elif "private" in access_modifier %}
    {% set call_class = global_tests_data.call_class.private %}
{% else %}
    {% set call_class = global_tests_data.call_class.public %}
{% endif %}

{% if "protected" in access_modifier %}
    {% set call_function = global_tests_data.call_function.protected %}
{% elif "private" in access_modifier %}
    {% set call_function = global_tests_data.call_function.private %}
{% else %}
    {% set call_function = global_tests_data.call_function.public %}
{% endif %}

{% if "static" in method_modifier %}
    {% set call_data = test_data.call_data.static %}
{% else %}
    {% set call_data = test_data.call_data.default %}
{% endif %}
{% set call_data = call_data|replace("[call_class]", call_class) %}
{% set call_data = call_data|replace("[call_function]", call_function) %}

{% set decl_data = test_data.decl_data ~ "\n" ~ (global_tests_data.global_decl_data|replace("[class_type_parameters]", test_data.class_type_parameters)) %}
{% set decl_data = decl_data|replace("[overload_method_data]", overload_method_data|indent(width=2)) %}
{% set decl_data = decl_data|replace("[modifiers_test_class_data]", modifiers_test_class_data|indent(width=2)) %}
{% set decl_data = decl_data|replace("[modifiers_decl_data]", modifiers_decl_data) %}

{{decl_data}}

function main(): void {
  {{call_data|indent(width=2)}}
}

{% endif %}
{% endfor %}
{% endfor %}
{% endfor %}

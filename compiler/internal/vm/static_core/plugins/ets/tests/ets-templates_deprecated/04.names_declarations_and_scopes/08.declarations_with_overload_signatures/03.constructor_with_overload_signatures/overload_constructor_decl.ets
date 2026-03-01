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
{% for class_modifier in global_tests_data.class_modifiers %}
{% for test_data in tests_data %}
/*---
desc: >-
    The syntax of a constructor with overload signatures is presented below (see also Constructor Declaration):
    constructorWithOverloadSignatures:
        constructorOverloadSignature*
        constructorDeclaration:;
    constructorOverloadSignature:
        accessModifier? 'constructor' signature;
---*/

{% set global_decl_data = global_tests_data.global_decl_data|replace("[class_type_parameters]", test_data.class_type_parameters) %}
{% if class_modifier == "" %}
    {% set global_decl_data = global_decl_data|replace("[class_modifier] ", "") %}
{% endif %}
{% set global_decl_data = global_decl_data|replace("[class_modifier]", class_modifier) %}

{% set overload_constructor_data = test_data.overload_constructor_data %}
{% if access_modifier == "" %}
    {% set overload_constructor_data = overload_constructor_data|replace("[access_modifier] ", "") %}
{% endif %}
{% set overload_constructor_data = overload_constructor_data|replace("[access_modifier]", access_modifier) %}

{% set ns = namespace(modifiers_constructor_variants="") %}
{% for overload_constructor_args_variant in test_data.overload_constructor_args_variants %}
    {% set mcv = "" %}
    {% if "private" in access_modifier %}
        {% set mcv = global_tests_data.modifiers_constructor_variants.private_constructor %}
    {% elif "protected" in access_modifier and "final" in class_modifier %}
        {% set mcv = global_tests_data.modifiers_constructor_variants.protected_constructor.final_class %}
    {% endif %}
    {% if mcv != "" %}
        {% if overload_constructor_args_variant.decl == "" %}
            {% set mcv = mcv|replace(", [overload_constructor_args_decl_variant]", "") %}
        {% endif %}
        {% set mcv = mcv|replace("[overload_constructor_args_decl_variant]", overload_constructor_args_variant.decl) %}
        {% set mcv = mcv|replace("[overload_constructor_args_call_variant]", overload_constructor_args_variant.call) %}
        {% if ns.modifiers_constructor_variants != "" %}
            {% set ns.modifiers_constructor_variants = ns.modifiers_constructor_variants ~ "\n" %}
        {% endif %}
        {% set ns.modifiers_constructor_variants = ns.modifiers_constructor_variants ~ mcv %}
    {% endif %}
{% endfor %}
{% set modifiers_constructor_variants = ns.modifiers_constructor_variants %}

{% set modifiers_decl_data = "" %}
{% set ns = namespace(derived_class_constructor_variants="") %}
{% set derived_class_constructor_variants_src_data = global_tests_data.modifiers_decl_data.derived_class_constructor_variants %}
{% for overload_constructor_args_variant in test_data.overload_constructor_args_variants %}
    {% set dccv = "" %}
    {% if "abstract" in class_modifier %}
        {% if "private" in access_modifier %}
            {% set dccv = derived_class_constructor_variants_src_data.abstract_class.private_constructor %}
        {% else %}
            {% set dccv = derived_class_constructor_variants_src_data.abstract_class.not_private_constructor %}
        {% endif %}
    {% elif class_modifier == "" and "protected" in access_modifier %}
        {% set dccv = derived_class_constructor_variants_src_data.class_without_class_modifiers.protected_constructor %}
    {% endif %}
    {% if dccv != "" %}
        {% if overload_constructor_args_variant.call == "" %}
            {% set dccv = dccv|replace(", [overload_constructor_args_call_variant]", "") %}
        {% endif %}
        {% set dccv = dccv|replace("[overload_constructor_args_decl_variant]", overload_constructor_args_variant.decl) %}
        {% set dccv = dccv|replace("[overload_constructor_args_call_variant]", overload_constructor_args_variant.call) %}
        {% if ns.derived_class_constructor_variants != "" %}
            {% set ns.derived_class_constructor_variants = ns.derived_class_constructor_variants ~ "\n" %}
        {% endif %}
        {% set ns.derived_class_constructor_variants = ns.derived_class_constructor_variants ~ dccv %}
    {% endif %}
{% endfor %}
{% set derived_class_constructor_variants = ns.derived_class_constructor_variants|indent(width=2) %}
{% if derived_class_constructor_variants != "" %}
    {% set modifiers_decl_data = "\n" ~ global_tests_data.modifiers_decl_data.derived_class_decl %}
    {% set modifiers_decl_data = modifiers_decl_data|replace("[derived_class_constructor_variants]", derived_class_constructor_variants) %}
    {% set modifiers_decl_data = modifiers_decl_data|replace("[class_type_parameters]", test_data.class_type_parameters) %}
{% endif %}

{% set call_class = global_tests_data.call_class.default %}
{% if "abstract" in class_modifier %}
    {% set call_class = global_tests_data.call_class.abstract_class %}
{% elif class_modifier == "" and "protected" in access_modifier %}
    {% set call_class = global_tests_data.call_class.class_without_class_modifiers.protected_constructor %}
{% endif %}

{% set add_args = "" %}
{% if "private" in access_modifier and "abstract" not in class_modifier %}
    {% set add_args = global_tests_data.add_args.private_constructor.not_abstract_class %}
{% elif "protected" in access_modifier and "final" in class_modifier %}
    {% set add_args = global_tests_data.add_args.protected_constructor.final_class %}
{% endif %}

{% set call_data = test_data.call_data %}
{% set call_data = call_data|replace("[call_class]", call_class) %}
{% if add_args == "" %}
    {% set call_data = call_data|replace("[add_args], ", "") %}
    {% set call_data = call_data|replace("[add_args],", "") %}
{% endif %}
{% set call_data = call_data|replace("[add_args]", add_args) %}

{% set decl_data = test_data.decl_data ~ "\n" ~ global_decl_data %}
{% set decl_data = decl_data|replace("[overload_constructor_data]", overload_constructor_data|indent(width=2)) %}
{% if modifiers_constructor_variants == "" %}
    {% set decl_data = decl_data|replace("\n  [modifiers_constructor_variants]", "") %}
{% endif %}
{% set decl_data = decl_data|replace("[modifiers_constructor_variants]", modifiers_constructor_variants|indent(width=2)) %}
{% if modifiers_decl_data == "" %}
    {% set decl_data = decl_data|replace("\n[modifiers_decl_data]", "") %}
{% endif %}
{% set decl_data = decl_data|replace("[modifiers_decl_data]", modifiers_decl_data) %}

{{decl_data}}

function main(): void {
  {{call_data|indent(width=2)}}
}

{% endfor %}
{% endfor %}
{% endfor %}

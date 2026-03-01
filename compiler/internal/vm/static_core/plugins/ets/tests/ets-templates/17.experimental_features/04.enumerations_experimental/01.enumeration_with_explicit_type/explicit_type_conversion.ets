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
desc: >-
    Value of enumeration type with explicit numeric base type
    is converted to the base type.
---*/
enum ExplicitTypeEnum: {{c.explicit_type}} {{c.init}}

function convert(value: {{c.explicit_type}}): Any { return value }

function main() {
    let explicit_type_enum_elem = ExplicitTypeEnum.a
    let explicit_type_value: {{c.explicit_type}} = explicit_type_enum_elem
    arktest.assertEQ(explicit_type_value, {{c.expected_value}}, "Equality check failed at " + explicit_type_value)
    let test_value = convert(ExplicitTypeEnum.a)
    arktest.assertTrue(test_value instanceof {{c.explicit_type}}, "Type check failed at " + test_value)
}
{%- endfor %}

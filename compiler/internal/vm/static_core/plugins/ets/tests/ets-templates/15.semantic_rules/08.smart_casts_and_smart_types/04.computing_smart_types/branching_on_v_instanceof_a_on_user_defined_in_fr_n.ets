/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

{% yaml "branching_on_v_instanceof_a_on_user_defined_common.yaml" as common_test_data %}

{% for c in common_test_data.cases %}

/*---
desc: >-
    Checking branching on "!(v instanceof A)" on user-defined types
    with "for" usage in function call and with smart type compatibility check
---*/

{{c.check_variant_decl_data}}

// Function for check that variable of computed smart type after branching on
// "!(v instanceof A)" is assignable to type T - A on the positive branch
function check_as_type_T_diff_A(v: T_diff_A,
                                v_id: int): void {
  check_values[v_id][2]!(v, v_id)
}
// Function for check values of variables of type T with branching on "!(v instanceof A)"
function check(v: T,
                v_type: VarType,
                v_id: int): void {
  for (;!(v instanceof A);) {
    arktest.assertEQ(v_type, VarType.T_diff_A)
    {{c.check_value_by_expression_on_the_negative_branch}}
    check_as_type_T_diff_A(v, v_id)
    return
  }
  arktest.assertEQ(v_type, VarType.T_inter_A)
}

function main(): void {
  let checks_number: int = 0
  for (let v_id: int = 0; v_id < check_values.length; v_id++) {
    check(check_values[v_id][0],
      check_values[v_id][3],
      v_id)
    checks_number++
  }
  arktest.assertEQ(checks_number, check_values_number)
}

{% endfor %}

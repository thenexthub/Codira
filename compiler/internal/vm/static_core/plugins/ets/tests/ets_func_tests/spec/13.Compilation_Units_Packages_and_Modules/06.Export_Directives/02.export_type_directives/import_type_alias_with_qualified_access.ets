/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

/*---
 desc: 13.07 Export Directives
 name: 13.compilation_units/13.07.export_directives/13.07.02.export_type_directives/import_type_alias_with_qualified_access_defect
 files: ['./to_export_for_import_type_alias_with_qualified_access.ets']
 tags:
 ---*/

import * as types from "./to_export_for_import_type_alias_with_qualified_access.ets"

function f1(): types.int32 {
  return 0;
}

function f2(): types.int16 {
  return 0;
}

function f3(): types.float_n {
  return 0.0f;
}

function f4(): types.text {
  return "test";
}

function f5(): types.flag {
  return true;
}

function main(): void {
  arktest.assertEQ(f1(), 0)
  arktest.assertEQ(f2(), 0)
  arktest.assertEQ(f3(), 0.0)
  arktest.assertEQ(f4(), "test")
  arktest.assertEQ(f5(), true)
}
